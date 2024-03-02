#include "Renderer.hpp"
#include "Graphics/pch.hpp"

#include "Renderer/Camera/CameraManager.hpp"
#include "Renderer/Components/BillboardRendererComponent.hpp"
#include "Renderer/Components/MeshRendererComponent.hpp"
#include "Renderer/Components/SkeletalMeshRendererComponent.hpp"
#include "Renderer/Components/TerrainRendererComponent.hpp"
#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/Rasterization/Light/ShadowedLightContext.hpp"
#include "Renderer/RendererManager.hpp"
#include "Renderer/Resources/TextureResource.hpp"
#include "Renderer/VertexTypes.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Graphics/AntiAliasing/TemporalEffect.hpp>
#include <Graphics/AmbientOcclusion/ScreenSpaceAmbientOcclusion.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsUtils/D3DUtils.hpp>
#include <Graphics/GraphicsUtils/RootSignature.hpp>
#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>
#include <Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Graphics/PostProcessing/MotionBlur.hpp>
#include <Graphics/PostProcessing/PostProcessing.hpp>
#include <Job/Job.hpp>
#include <Math/VectorMath.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Assert.hpp>

#ifdef _D_EDITOR
#include <imgui.h>

#endif // _D_EDITOR

#include <filesystem>

using namespace D_CONTAINERS;
using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_GRAPHICS;
using namespace D_GRAPHICS_MEMORY;
using namespace D_GRAPHICS_UTILS;
using namespace D_RENDERER;
using namespace D_RENDERER_GEOMETRY;
using namespace D_RESOURCE;
using namespace Microsoft::WRL;

#define VertexData(il) il::InputLayout.NumElements, il::InputLayout.pInputElementDescs
#define ShaderData(name) D_GRAPHICS::GetShaderByName(name)->GetBufferPointer(), D_GRAPHICS::GetShaderByName(name)->GetBufferSize()

namespace Darius::Renderer::Rasterization
{
	bool												_initialized = false;

	// Input layout and root signature
	std::array<D_GRAPHICS_UTILS::RootSignature, (size_t)RootSignatureTypes::_numRootSig> RootSigns;
	DVector<D_GRAPHICS_UTILS::GraphicsPSO> Psos;

	DescriptorHandle									CommonTexture;

	D_RESOURCE::ResourceRef<TextureResource>			RadianceCubeMap;
	D_RESOURCE::ResourceRef<TextureResource>			IrradianceCubeMap;
	D_RESOURCE::ResourceRef<TextureResource>			DefaultBlackCubeMap;
	float												SpecularIBLRange;
	float												SpecularIBLBias = FLT_MAX;

	GraphicsPSO											DefaultPso;
	GraphicsPSO											SkyboxPso;

	UINT16												ForcedPsoFlags;

	// Heaps
	DescriptorHeap										TextureHeap;
	DescriptorHeap										SamplerHeap;

	std::unique_ptr<D_RENDERER_RAST_LIGHT::RasterizationShadowedLightContext> LightContext;

	//////////////////////////////////////////////////////
	// Options
	bool												SeparateZPass = true;

	//////////////////////////////////////////////////////
	// Functions
	void BuildDefaultPSOs();
	void BuildRootSignature();
	void DrawSkybox(D_GRAPHICS::GraphicsContext& context, const D_MATH_CAMERA::BaseCamera& camera, D_GRAPHICS_BUFFERS::ColorBuffer& sceneColor, D_GRAPHICS_BUFFERS::DepthBuffer& sceneDepth, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor);


	void Clear(D_GRAPHICS::GraphicsContext& context, D_GRAPHICS_BUFFERS::ColorBuffer& rt, D_GRAPHICS_BUFFERS::DepthBuffer& depthStencil, RECT bounds, std::wstring const& processName = L"Clear");


	void Initialize(D_SERIALIZATION::Json const& settings)
	{

		D_ASSERT(!_initialized);

		D_H_OPTIONS_LOAD_BASIC("Renderer.Rasterization.Passes.SeparateZ", SeparateZPass);

		BuildRootSignature();
		BuildDefaultPSOs();

		// Creating heaps
		TextureHeap.Create(L"Rasterization SRV, UAV, CBV  Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
		SamplerHeap.Create(L"Rasterization Sampler  Descriptor Heap", D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);

		CommonTexture = TextureHeap.Alloc(8);

		DefaultBlackCubeMap = D_RESOURCE::GetResourceSync<TextureResource>(GetDefaultGraphicsResource(D_RENDERER::DefaultResource::TextureCubeMapBlack));

		// Initialize IBL Textrues on GPU
		{
			uint32_t DestCount = 2;
			uint32_t SourceCounts[] = { 1, 1 };

			D3D12_CPU_DESCRIPTOR_HANDLE specHandle = DefaultBlackCubeMap->GetTextureData()->GetSRV();
			D3D12_CPU_DESCRIPTOR_HANDLE diffHandle = DefaultBlackCubeMap->GetTextureData()->GetSRV();

			D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
			{
				specHandle,
				diffHandle
			};

			for (int i = 0; i < D_GRAPHICS_DEVICE::gNumFrameResources; i++)
			{
				DescriptorHandle dest = CommonTexture + 6 * TextureHeap.GetDescriptorSize();

				D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &dest, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
		}

		// Initializing Camera Manager
		D_CAMERA_MANAGER::Initialize();

		LightContext = std::make_unique<D_RENDERER_RAST_LIGHT::RasterizationShadowedLightContext>();

		ForcedPsoFlags = 0u;
	}

	void Shutdown()
	{
		LightContext.reset();

		D_CAMERA_MANAGER::Shutdown();

		TextureHeap.Destroy();
		SamplerHeap.Destroy();
	}

	void Update(D_GRAPHICS::CommandContext& context)
	{

		{
			D_PROFILING::ScopedTimer _prof(L"Update Renderer Components", context);

			D_CAMERA_MANAGER::Update();

			DVector<std::function<void()>> updateFuncs;

			auto componentCount = 0u;
			componentCount += D_WORLD::CountComponents<MeshRendererComponent>();
			componentCount += D_WORLD::CountComponents<SkeletalMeshRendererComponent>();
			componentCount += D_WORLD::CountComponents<BillboardRendererComponent>();
			componentCount += D_WORLD::CountComponents<TerrainRendererComponent>();
			updateFuncs.reserve(componentCount);

#define RENDERER_COMPONENT_UPDATE_ITER(T) \
			D_WORLD::IterateComponents<T>([&](D_RENDERER::T& meshComp) \
				{ \
					updateFuncs.push_back([&]() \
						{ \
							if(meshComp.IsActive()) \
								meshComp.Update(-1.f); \
						}); \
				} \
			) \

			RENDERER_COMPONENT_UPDATE_ITER(MeshRendererComponent);
			RENDERER_COMPONENT_UPDATE_ITER(SkeletalMeshRendererComponent);
			RENDERER_COMPONENT_UPDATE_ITER(BillboardRendererComponent);
			RENDERER_COMPONENT_UPDATE_ITER(TerrainRendererComponent);
#undef RENDERER_COMPONENT_UPDATE_ITER

			D_JOB::AddTaskSetAndWait(updateFuncs);
		}
	}

	void AddRenderItems(D_RENDERER_RAST::MeshSorter& sorter, D_MATH_CAMERA::BaseCamera const& cam, RenderItemContext const& riContext)
	{
		auto frustum = cam.GetViewSpaceFrustum();

		// Iterating over static meshes
#define ADD_RENDERER_COMPONENT_RENDER_ITEMS(type) \
		D_WORLD::IterateComponents<type>([&](D_RENDERER::type& meshComp) \
			{ \
				/* Can't render */ \
				if (!meshComp.CanRender()) \
					return; \
\
				/* Is it in our frustum */ \
				auto sphereWorldSpace = AffineTransform(meshComp.GetTransform()->GetWorld()) * meshComp.GetBounds(); \
				auto sphereViewSpace = BoundingSphere(Vector3(cam.GetViewMatrix() * sphereWorldSpace.GetCenter()), sphereWorldSpace.GetRadius()); \
				if (!frustum.Intersects(sphereViewSpace)) \
					return; \
\
				auto distance = -sphereViewSpace.GetCenter().GetZ() - sphereViewSpace.GetRadius(); \
\
				meshComp.AddRenderItems([distance, &sorter](auto const& ri) \
					{ \
						sorter.AddMesh(ri, distance); \
					}, riContext); \
			}); \

		ADD_RENDERER_COMPONENT_RENDER_ITEMS(MeshRendererComponent);
		ADD_RENDERER_COMPONENT_RENDER_ITEMS(SkeletalMeshRendererComponent);
		ADD_RENDERER_COMPONENT_RENDER_ITEMS(BillboardRendererComponent);
		ADD_RENDERER_COMPONENT_RENDER_ITEMS(TerrainRendererComponent);

#undef ADD_RENDERER_COMPONENT_RENDER_ITEMS

	}

	void AddShadowRenderItems(D_CONTAINERS::DVector<RenderItem>& items, RenderItemContext const& riContext)
	{

		RenderItemContext shadowRiContext = riContext;
		shadowRiContext.Shadow = true;

		UINT shadowCompsCount = 0;
		shadowCompsCount += D_WORLD::CountComponents<D_RENDERER::MeshRendererComponent>();
		shadowCompsCount += D_WORLD::CountComponents<D_RENDERER::SkeletalMeshRendererComponent>();
		shadowCompsCount += D_WORLD::CountComponents<D_RENDERER::BillboardRendererComponent>();
		shadowCompsCount += D_WORLD::CountComponents<D_RENDERER::TerrainRendererComponent>();
		items.reserve(shadowCompsCount);


#define ADD_RENDERER_COMPONENT_RENDER_ITMES(type) \
		/* Iterating over meshes */ \
		D_WORLD::IterateComponents<type>([&](D_RENDERER::type& meshComp) \
			{ \
				/* Can't render */ \
				if (!meshComp.CanRender()) \
					return; \
 \
				if (!meshComp.IsCastingShadow()) \
					return; \
\
				meshComp.AddRenderItems([&items](auto const& ri) \
					{ \
						auto item = ri; \
						item.Material.SamplersSRV.ptr = 0; \
						items.push_back(item); \
					}, shadowRiContext); \
			}); \

		ADD_RENDERER_COMPONENT_RENDER_ITMES(MeshRendererComponent);
		ADD_RENDERER_COMPONENT_RENDER_ITMES(SkeletalMeshRendererComponent);
		ADD_RENDERER_COMPONENT_RENDER_ITMES(BillboardRendererComponent);
		ADD_RENDERER_COMPONENT_RENDER_ITMES(TerrainRendererComponent);

#undef ADD_RENDERER_COMPONENT_RENDER_ITMES
	}

	void Render(std::wstring const& jobId, SceneRenderContext& rContext, std::function<void()> postAntiAliasing)
	{
		auto& context = rContext.CommandContext.GetGraphicsContext();
		D_PROFILING::ScopedTimer _prof(L"Rasterization Render", context);

		// Update Lights
		{
			D_PROFILING::ScopedTimer _prof(L"Update Shadowed Light Context", context);

			LightContext->Update(rContext.Camera);
			LightContext->UpdateBuffers(context, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}

		MeshSorter sorter(MeshSorter::kDefault);

		{
			D_PROFILING::ScopedTimer _prof1(L"Clearing render targets", context);
			// Clearing depth and scene color textures
			context.TransitionResource(rContext.DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			if (rContext.CustomDepthBuffer)
				context.TransitionResource(*rContext.CustomDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);

			context.TransitionResource(rContext.ColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
			context.TransitionResource(rContext.NormalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			context.ClearDepth(rContext.DepthBuffer);
			if (D_GRAPHICS::IsStencilEnable())
				context.ClearStencil(rContext.DepthBuffer);
			if (rContext.CustomDepthBuffer)
				context.ClearDepth(*rContext.CustomDepthBuffer);
			context.ClearColor(rContext.ColorBuffer);
			context.ClearColor(rContext.NormalBuffer);
		}

		auto width = rContext.ColorBuffer.GetWidth();
		auto height = rContext.ColorBuffer.GetHeight();

		// Setting up sorter
		auto viewPort = CD3DX12_VIEWPORT(0.f, 0.f, (float)width, (float)height, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissor = CD3DX12_RECT(0l, 0l, (long)width, (long)height);
		sorter.SetCamera(rContext.Camera);
		sorter.SetViewport(viewPort);
		sorter.SetScissor(scissor);
		sorter.SetDepthStencilTarget(rContext.DepthBuffer, rContext.CustomDepthBuffer);
		sorter.AddRenderTarget(rContext.ColorBuffer);
		sorter.SetNormalTarget(rContext.NormalBuffer);

		// Add meshes to sorter
		AddRenderItems(sorter, rContext.Camera, rContext.RenderItemContext);

		{
			// Creating shadows

			DVector<RenderItem> shadowRenderItems;
			AddShadowRenderItems(shadowRenderItems, rContext.RenderItemContext);

			LightContext->RenderShadows(shadowRenderItems, context);
		}

		sorter.Sort();

		if (rContext.DrawSkybox)
		{
			D_RENDERER_RAST::SetIBLTextures(rContext.IrradianceIBL, rContext.RadianceIBL);
			DrawSkybox(context, rContext.Camera, rContext.ColorBuffer, rContext.DepthBuffer, viewPort, scissor);
		}

		// Rendering depth
		sorter.RenderMeshes(MeshSorter::kZPass, context, 0, rContext.Globals);

		D_GRAPHICS_AO_SS::SSAORenderBuffers ssaoBuffers =
		{
			rContext.ColorBuffer,
			rContext.SSAOFullScreen,
			rContext.DepthBuffer,
			*rContext.LinearDepth,
			rContext.DepthDownsize1,
			rContext.DepthDownsize2,
			rContext.DepthDownsize3,
			rContext.DepthDownsize4,
			rContext.DepthTiled1,
			rContext.DepthTiled2,
			rContext.DepthTiled3,
			rContext.DepthTiled4,
			rContext.AOMerged1,
			rContext.AOMerged2,
			rContext.AOMerged3,
			rContext.AOMerged4,
			rContext.AOSmooth1,
			rContext.AOSmooth2,
			rContext.AOSmooth3,
			rContext.AOHighQuality1,
			rContext.AOHighQuality2,
			rContext.AOHighQuality3,
			rContext.AOHighQuality4
		};

		D_GRAPHICS_AO_SS::Render(context, ssaoBuffers, rContext.Camera);


		sorter.RenderMeshes(MeshSorter::kTransparent, context, &rContext.SSAOFullScreen, rContext.Globals);

		auto frameIdxMod2 = D_GRAPHICS_AA_TEMPORAL::GetFrameIndexMod2();
		auto& commandContext = context.GetComputeContext();

		D_GRAPHICS_PP_MOTION::MotionBlurBuffers motionBuffers = { rContext.ColorBuffer, rContext.LinearDepth[frameIdxMod2], rContext.VelocityBuffer, rContext.DepthBuffer };

		D_GRAPHICS_PP_MOTION::GenerateCameraVelocityBuffer(commandContext, motionBuffers, rContext.Camera);

		D_GRAPHICS_AA_TEMPORAL::ResolveImage(commandContext, rContext.ColorBuffer, rContext.VelocityBuffer, rContext.TemporalColor, rContext.LinearDepth);


		// Calling post anti-aliasing callback
		if (postAntiAliasing)
			postAntiAliasing();

		// Additional renders
		if (rContext.AdditionalRenderItems.size() > 0)
		{
			context.TransitionResource(rContext.ColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

			MeshSorter additionalRenderSorter(sorter);

			for (auto const& additionalItemsVec : rContext.AdditionalRenderItems)
			{
				for (auto& ri : *additionalItemsVec)
				{
					additionalRenderSorter.AddMesh(ri, -1);
				}
			}

			additionalRenderSorter.Sort();

			additionalRenderSorter.RenderMeshes(MeshSorter::kTransparent, context, nullptr, rContext.Globals);
		}
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		D_H_OPTION_DRAW_CHECKBOX("Separate Z Pass", "Renderer.Rasterization.Passes.SeparateZ", SeparateZPass);

		D_H_OPTION_DRAW_END();
	}
#endif

	D_GRAPHICS_UTILS::RootSignature& GetRootSignature(RootSignatureTypes type)
	{
		return RootSigns[(size_t)type];
	}

	// Helper method to clear the back buffers.
	void Clear(D_GRAPHICS::GraphicsContext& context, D_GRAPHICS_BUFFERS::ColorBuffer& rt, D_GRAPHICS_BUFFERS::DepthBuffer& depthStencil, RECT bounds, std::wstring const& processName)
	{

		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, processName.c_str());

		// Clear the views.
		auto const rtvDescriptor = rt.GetRTV();
		auto const dsvDescriptor = depthStencil.GetDSV();

		// Set the viewport and scissor rect.
		long width = bounds.right - bounds.left;
		long height = bounds.bottom - bounds.top;
		auto viewport = CD3DX12_VIEWPORT((float)bounds.left, (float)bounds.top, (float)width, (float)height, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissorRect = CD3DX12_RECT(bounds.left, bounds.top, (long)width, (long)height);

		context.ClearColor(rt, &scissorRect);
		context.ClearDepth(depthStencil);
		context.SetRenderTarget(rtvDescriptor, dsvDescriptor);
		context.SetViewportAndScissor(viewport, scissorRect);

		PIXEndEvent(context.GetCommandList());
	}

	void BuildDefaultPSOs()
	{

		DXGI_FORMAT ColorFormat = D_GRAPHICS::GetColorFormat();
		DXGI_FORMAT DepthFormat = D_GRAPHICS::GetDepthFormat();
		DXGI_FORMAT ShadowFormat = D_GRAPHICS::GetShadowFormat();

		DXGI_FORMAT rtFormats[] = { D_GRAPHICS::GetColorFormat(), DXGI_FORMAT_R16G16B16A16_FLOAT }; // Color and normal

		// For Opaque objects
		DefaultPso = GraphicsPSO(L"Opaque PSO");
		DefaultPso.SetInputLayout(VertexData(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture));
		DefaultPso.SetVertexShader(ShaderData("DefaultVS"));
		DefaultPso.SetPixelShader(ShaderData("DefaultPS"));
		DefaultPso.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		DefaultPso.SetRasterizerState(D_GRAPHICS::RasterizerDefault);
		DefaultPso.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		DefaultPso.SetDepthStencilState(DepthStateReadWrite);
		DefaultPso.SetSampleMask(UINT_MAX);
		DefaultPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		DefaultPso.SetRenderTargetFormats(2, rtFormats, DepthFormat);
		DefaultPso.Finalize();

		// Skybox
		{
			SkyboxPso = DefaultPso;
			SkyboxPso.SetDepthStencilState(DepthStateTestEqual);
			SkyboxPso.SetInputLayout(0, nullptr);
			SkyboxPso.SetVertexShader(ShaderData("SkyboxVS"));
			SkyboxPso.SetPixelShader(ShaderData("SkyboxPS"));
			SkyboxPso.Finalize(L"Skybox");
		}

		Psos.push_back(GraphicsPSO(L"Invalid PSO"));
	}

	void BuildRootSignature()
	{
		// Default root signature
		auto& def = RootSigns[(size_t)RootSignatureTypes::DefaultRootSig];
		def.Reset(kNumRootBindings, 5);

		// Create samplers
		SamplerDesc defaultSamplerDesc;
		defaultSamplerDesc.MaxAnisotropy = 8;
		SamplerDesc cubeMapSamplerDesc = defaultSamplerDesc;
		def.InitStaticSampler(10, defaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(11, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(12, cubeMapSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(13, SamplerLinearWrapDesc);
		def.InitStaticSampler(14, SamplerLinearClampDesc);

		// Create root CBVs.
		def[kMeshConstantsVS].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
		def[kMeshConstantsHS].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_HULL);
		def[kMeshConstantsDS].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_DOMAIN);
		def[kDomainConstantsDs].InitAsConstantBuffer(2, D3D12_SHADER_VISIBILITY_DOMAIN);
		def[kMaterialConstantsPs].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kTextureDsSRVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10, D3D12_SHADER_VISIBILITY_DOMAIN);
		def[kMaterialSRVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kMaterialSamplers].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 10, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kCommonCBV].InitAsConstantBuffer(1);
		def[kCommonSRVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 8, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kSkinMatrices].InitAsBufferSRV(20, D3D12_SHADER_VISIBILITY_VERTEX);
		def.Finalize(L"Main Root Sig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}

	void SetIBLTextures(D_RENDERER::TextureResource* diffuseIBL, D_RENDERER::TextureResource* specularIBL)
	{

		bool loadIrradiance = false;
		bool loadRadiance = false;

		// Set default texture if ibl textures are not loaded and
		// have them loaded if necessary
		if (diffuseIBL == nullptr || !diffuseIBL->GetTextureData()->IsCubeMap())
		{
			IrradianceCubeMap = DefaultBlackCubeMap;
		}
		else if (!diffuseIBL->IsLoaded() || diffuseIBL->IsDirtyGPU())
		{
			IrradianceCubeMap = DefaultBlackCubeMap;
			loadIrradiance = true;
		}
		else
		{
			IrradianceCubeMap = diffuseIBL;
		}

		if (specularIBL == nullptr || !specularIBL->GetTextureData()->IsCubeMap())
		{
			RadianceCubeMap = DefaultBlackCubeMap;
		}
		else if (!specularIBL->IsLoaded() || specularIBL->IsDirtyGPU())
		{
			RadianceCubeMap = DefaultBlackCubeMap;
			loadRadiance = true;
		}
		else
		{
			RadianceCubeMap = specularIBL;
		}


		// Set radiance cube map function
		auto setRadiance = [](TextureResource* specular)
			{
				auto texRes = const_cast<ID3D12Resource*>(RadianceCubeMap->GetTextureData()->GetResource());
				const D3D12_RESOURCE_DESC& texDesc = texRes->GetDesc();
				SpecularIBLRange = D_MATH::Max(0.f, (float)texDesc.MipLevels - 1);
				SpecularIBLBias = D_MATH::Min(SpecularIBLBias, SpecularIBLRange);

				UINT destCount = 1;
				UINT sourceCounts = 1;
				D3D12_CPU_DESCRIPTOR_HANDLE texHandle = RadianceCubeMap->GetTextureData()->GetSRV();

				DescriptorHandle dest = CommonTexture + 6 * TextureHeap.GetDescriptorSize();
				D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &dest, &destCount, destCount, &texHandle, &sourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			};

		// Set irradiance cube map function
		auto setIrradiance = [](TextureResource* irradiance)
			{
				UINT destCount = 1;
				UINT sourceCounts = 1;
				D3D12_CPU_DESCRIPTOR_HANDLE texHandle = IrradianceCubeMap->GetTextureData()->GetSRV();

				DescriptorHandle dest = CommonTexture + 7 * TextureHeap.GetDescriptorSize();
				D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &dest, &destCount, destCount, &texHandle, &sourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

			};

		if (loadRadiance)
		{
			D_RESOURCE::ResourceLoader::LoadResourceAsync(specularIBL, nullptr, true);
		}
		else
		{
			setRadiance(specularIBL);
		}

		if (loadIrradiance)
		{
			D_RESOURCE::ResourceLoader::LoadResourceAsync(diffuseIBL, nullptr, true);
		}
		else
		{
			setIrradiance(diffuseIBL);
		}
	}

	void DrawSkybox(D_GRAPHICS::GraphicsContext& context, const D_MATH_CAMERA::BaseCamera& camera, D_GRAPHICS_BUFFERS::ColorBuffer& sceneColor, D_GRAPHICS_BUFFERS::DepthBuffer& sceneDepth, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor)
	{
		D_PROFILING::ScopedTimer _prof(L"Draw Skybox", context);

		// Creating constant buffers
		ALIGN_DECL_16 struct SkyboxVSCB
		{
			Matrix4 ProjInverse;
			Matrix3 ViewInverse;
		} skyVSCB;
		skyVSCB.ProjInverse = Invert(camera.GetProjMatrix());
		skyVSCB.ViewInverse = Invert(camera.GetViewMatrix()).Get3x3();

		ALIGN_DECL_16 struct SkyboxPSCB
		{
			float TextureLevel;
		} skyPSVB;
		skyPSVB.TextureLevel = SpecularIBLBias;

		// Setting root signature and pso
		context.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		context.SetPipelineState(SkyboxPso);

		// Transition of render targets
		context.TransitionResource(sceneDepth, D3D12_RESOURCE_STATE_DEPTH_READ);
		context.TransitionResource(sceneColor, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		context.SetRenderTarget(sceneColor.GetRTV(), sceneDepth.GetDSV_DepthReadOnly());
		context.SetViewportAndScissor(viewport, scissor);

		// Bind pipeline resources
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TextureHeap.GetHeapPointer());
		context.SetDynamicConstantBufferView(kMeshConstantsVS, sizeof(SkyboxVSCB), &skyVSCB);
		context.SetDynamicConstantBufferView(kMaterialConstantsPs, sizeof(SkyboxPSCB), &skyPSVB);
		context.SetDescriptorTable(kCommonSRVs, CommonTexture);

		context.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		context.Draw(3);
	}

	void SetIBLBias(float LODBias)
	{
		SpecularIBLBias = Min(LODBias, SpecularIBLRange);
	}

	void MeshSorter::AddMesh(RenderItem const& renderItem, float distance)
	{
		SortKey key;
		key.value = m_SortObjects.size();

		bool alphaBlend = (renderItem.PsoFlags & RenderItem::AlphaBlend) == RenderItem::AlphaBlend;
		bool alphaTest = (renderItem.PsoFlags & RenderItem::AlphaTest) == RenderItem::AlphaTest;

		union float_or_int { float f; uint32_t u; } dist;
		dist.f = Max(distance, 0.0f);

		if (m_BatchType == kShadows)
		{
			if (alphaBlend)
				return;

			UINT shadowDepthPSO = renderItem.DepthPsoIndex > 0 ? renderItem.DepthPsoIndex + 1 : GetPso({ renderItem.PsoFlags, 0 }) + 1;

			key.passID = kZPass;
			key.psoIdx = shadowDepthPSO;
			key.key = dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kZPass]++;
		}
		else if (renderItem.PsoFlags & RenderItem::AlphaBlend)
		{
			key.passID = kTransparent;
			key.psoIdx = renderItem.PsoType;
			key.key = ~(uint64_t)dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kTransparent]++;
		}
		else if (SeparateZPass || alphaTest)
		{
			UINT16 flags = renderItem.PsoFlags | RenderItem::DepthOnly;
			UINT depthPSO = renderItem.DepthPsoIndex > 0 ? renderItem.DepthPsoIndex : GetPso({ flags, 0u });

			key.passID = kZPass;
			key.psoIdx = depthPSO;
			key.key = dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kZPass]++;

			key.passID = kOpaque;
			key.psoIdx = renderItem.PsoType + 1;
			key.key = dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kOpaque]++;
		}
		else
		{
			key.passID = kOpaque;
			key.psoIdx = renderItem.PsoType;
			key.key = dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kOpaque]++;

		}

		SortObject object = { renderItem };
		m_SortObjects.push_back(object);
	}

	void MeshSorter::Sort()
	{
		struct { bool operator()(uint64_t a, uint64_t b) const { return a < b; } } Cmp;
		std::sort(m_SortKeys.begin(), m_SortKeys.end(), Cmp);
	}

	void MeshSorter::RenderMeshes(
		DrawPass pass,
		D_GRAPHICS::GraphicsContext& context,
		D_GRAPHICS_BUFFERS::ColorBuffer* ssao,
		GlobalConstants& globals)
	{
		D_ASSERT(m_DSV != nullptr);

		context.PIXBeginEvent(L"Render Meshes");

		context.SetRootSignature(RootSigns[DefaultRootSig]);

		globals.IBLBias = SpecularIBLBias;
		globals.IBLRange = SpecularIBLRange;
		context.SetDynamicConstantBufferView(kCommonCBV, sizeof(GlobalConstants), &globals);

		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TextureHeap.GetHeapPointer());
		context.SetDescriptorTable(kCommonSRVs, CommonTexture);

		if (m_BatchType == kShadows)
		{
			context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
			context.ClearDepth(*m_DSV);
			context.SetDepthStencilTarget(m_DSV->GetDSV());

			if (m_Viewport.Width == 0)
			{
				m_Viewport.TopLeftX = 0.0f;
				m_Viewport.TopLeftY = 0.0f;
				m_Viewport.Width = (float)m_DSV->GetWidth();
				m_Viewport.Height = (float)m_DSV->GetHeight();
				m_Viewport.MaxDepth = 1.0f;
				m_Viewport.MinDepth = 0.0f;

				m_Scissor.left = 0;
				m_Scissor.right = m_DSV->GetWidth();
				m_Scissor.top = 0;
				m_Scissor.bottom = m_DSV->GetHeight();
			}
		}
		else
		{
			// Copying light data
			if (pass >= kOpaque)
			{
				UINT destCount = ssao ? 6 : 5;
				UINT sourceCounts[] = { 1, 1, 1, 1, 1, 1 };
				D3D12_CPU_DESCRIPTOR_HANDLE lightHandles[] =
				{
					LightContext->GetLightsStatusBufferDescriptor(),
					LightContext->GetLightsDataBufferDescriptor(),
					(D3D12_CPU_DESCRIPTOR_HANDLE)0,
					(D3D12_CPU_DESCRIPTOR_HANDLE)0,
					(D3D12_CPU_DESCRIPTOR_HANDLE)0,
					ssao ? ssao->GetSRV() : (D3D12_CPU_DESCRIPTOR_HANDLE)0
				};

				LightContext->GetShadowTextureArraysHandles(lightHandles[2], lightHandles[3], lightHandles[4]);

				D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &CommonTexture, &destCount, destCount, lightHandles, sourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			// Setup samplers
			context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SamplerHeap.GetHeapPointer());

			for (uint32_t i = 0; i < m_NumRTVs; ++i)
			{
				D_ASSERT(m_RTV[i] != nullptr);
				D_ASSERT(m_DSV->GetWidth() == m_RTV[i]->GetWidth());
				D_ASSERT(m_DSV->GetHeight() == m_RTV[i]->GetHeight());
			}

			if (m_Viewport.Width == 0)
			{
				m_Viewport.TopLeftX = 0.0f;
				m_Viewport.TopLeftY = 0.0f;
				m_Viewport.Width = (float)m_DSV->GetWidth();
				m_Viewport.Height = (float)m_DSV->GetHeight();
				m_Viewport.MaxDepth = 1.0f;
				m_Viewport.MinDepth = 0.0f;

				m_Scissor.left = 0;
				m_Scissor.right = m_DSV->GetWidth();
				m_Scissor.top = 0;
				m_Scissor.bottom = m_DSV->GetWidth();
			}
		}

		for (; m_CurrentPass <= pass; m_CurrentPass = (DrawPass)(m_CurrentPass + 1))
		{
			const uint32_t passCount = m_PassCounts[m_CurrentPass];
			if (passCount == 0)
				continue;

			bool customDepthWriteAvailable = false;

			if (m_BatchType == kDefault)
			{
				switch (m_CurrentPass)
				{
				case kZPass:
					context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					context.SetDepthStencilTarget(m_DSV->GetDSV());

					if (m_DSVCustom)
					{
						context.TransitionResource(*m_DSVCustom, D3D12_RESOURCE_STATE_DEPTH_WRITE);
						customDepthWriteAvailable = true;
					}

					break;
				case kOpaque:
					if (SeparateZPass)
					{
						context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_READ);
						context.TransitionResource(*m_RTV[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
						context.TransitionResource(*m_Norm, D3D12_RESOURCE_STATE_RENDER_TARGET);
						D3D12_CPU_DESCRIPTOR_HANDLE RTs[] = { m_RTV[0]->GetRTV(), m_Norm->GetRTV() };
						context.SetRenderTargets(2, RTs, m_DSV->GetDSV_DepthReadOnly());
					}
					else
					{
						context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE);
						if (m_DSVCustom)
						{
							context.TransitionResource(*m_DSVCustom, D3D12_RESOURCE_STATE_DEPTH_WRITE);
							customDepthWriteAvailable = true;
						}
						context.TransitionResource(*m_RTV[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
						context.TransitionResource(*m_Norm, D3D12_RESOURCE_STATE_RENDER_TARGET);
						D3D12_CPU_DESCRIPTOR_HANDLE RTs[] = { m_RTV[0]->GetRTV(), m_Norm->GetRTV() };
						context.SetRenderTargets(2, RTs, m_DSV->GetDSV());
						//context.SetRenderTarget(m_RTV[0]->GetRTV(), m_DSV->GetDSV());
					}
					break;
				case kTransparent:
					context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_READ);
					context.TransitionResource(*m_RTV[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
					context.SetRenderTarget(m_RTV[0]->GetRTV(), m_DSV->GetDSV_DepthReadOnly());
					break;
				}
			}

			context.SetViewportAndScissor(m_Viewport, m_Scissor);
			context.FlushResourceBarriers();

			const uint32_t lastDraw = m_CurrentDraw + passCount;

			bool dirtyRenderTarget = false;

			while (m_CurrentDraw < lastDraw)
			{
				SortKey key;
				key.value = m_SortKeys[m_CurrentDraw];
				const SortObject& object = m_SortObjects[key.objectIdx];
				RenderItem const& ri = object.renderItem;

				if (dirtyRenderTarget)
					SetupDefaultBatchTypeRenderTargetsAfterCustomDepth(context);

				if (ri.MeshVsCBV != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
					context.SetConstantBuffer(kMeshConstantsVS, ri.MeshVsCBV);

				if (ri.MeshHsCBV != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
					context.SetConstantBuffer(kMeshConstantsHS, ri.MeshHsCBV);

				if (ri.MeshDsCBV != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
					context.SetConstantBuffer(kMeshConstantsDS, ri.MeshDsCBV);

				if (ri.ParamsDsCBV != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
					context.SetConstantBuffer(kDomainConstantsDs, ri.ParamsDsCBV);

				if (ri.PsoFlags & RenderItem::ColorOnly)
				{
					D_RENDERER::ColorConstants color = { ri.Color };
					context.SetDynamicConstantBufferView(kMaterialConstantsPs, sizeof(ColorConstants), &color);
				}
				else
				{
					context.SetConstantBuffer(kMaterialConstantsPs, ri.Material.MaterialCBV);
					context.SetDescriptorTable(kMaterialSRVs, ri.Material.MaterialSRV);

					if (ri.Material.SamplersSRV.ptr != 0)
						context.SetDescriptorTable(kMaterialSamplers, ri.Material.SamplersSRV);
				}

				if (ri.TextureDomainSRV.ptr != 0)
				{
					context.SetDescriptorTable(kTextureDsSRVs, ri.TextureDomainSRV);
				}

				if (ri.mNumJoints > 0)
				{
					context.SetDynamicSRV(kSkinMatrices, sizeof(Joint) * ri.mNumJoints, ri.mJointData);
				}

				context.SetPipelineState(Psos[key.psoIdx]);

				context.SetPrimitiveTopology(ri.PrimitiveType);

				if (!(ri.PsoFlags & RenderItem::SkipVertexIndex))
				{
					context.SetVertexBuffer(0, ri.Mesh->VertexBufferView());
					context.SetIndexBuffer(ri.Mesh->IndexBufferView());
				}

				if (m_CurrentPass == kZPass)
				{
					if (ri.StencilEnable)
						context.SetStencilRef(ri.StencilValue);
					else
						context.SetStencilRef(0u);
				}

				// Main render
				if (ri.PsoFlags & RenderItem::SkipVertexIndex)
					context.DrawInstanced(ri.IndexCount, 1, ri.BaseVertexLocation, 0);
				else
					context.DrawIndexedInstanced(ri.IndexCount, 1, ri.StartIndexLocation, ri.BaseVertexLocation, 0);

				// Custom depth
				if (ri.CustomDepth && customDepthWriteAvailable)
				{
					context.SetDepthStencilTarget(m_DSVCustom->GetDSV());
					if (ri.PsoFlags & RenderItem::SkipVertexIndex)
						context.DrawInstanced(ri.IndexCount, 1, ri.BaseVertexLocation, 0);
					else
						context.DrawIndexedInstanced(ri.IndexCount, 1, ri.StartIndexLocation, ri.BaseVertexLocation, 0);
					dirtyRenderTarget = true;
				}

				++m_CurrentDraw;
			}
		}

		context.PIXEndEvent();
	}

	void MeshSorter::SetupDefaultBatchTypeRenderTargetsAfterCustomDepth(D_GRAPHICS::GraphicsContext& context)
	{
		switch (m_CurrentPass)
		{
		case kZPass:
			context.SetDepthStencilTarget(m_DSV->GetDSV());
			break;
		case kOpaque:
			if (!SeparateZPass)
			{
				D3D12_CPU_DESCRIPTOR_HANDLE RTs[] = { m_RTV[0]->GetRTV(), m_Norm->GetRTV() };
				context.SetRenderTargets(2, RTs, m_DSV->GetDSV());
			}
			break;
		}
	}

#ifdef _D_EDITOR

	UINT16 const& GetForcedPsoFlags()
	{
		return ForcedPsoFlags;
	}

	void SetForceWireframe(bool val)
	{
		if (val)
			ForcedPsoFlags |= RenderItem::Wireframe;
		else
			ForcedPsoFlags &= ~RenderItem::Wireframe;

	}

#endif

	UINT GetDepthOnlyPso(PsoConfig const& _psoConfig)
	{
		static const uint16_t relevantFlags = RenderItem::AlphaTest | RenderItem::HasSkin | RenderItem::TwoSided | RenderItem::Wireframe | RenderItem::LineOnly | RenderItem::SkipVertexIndex | RenderItem::PointOnly | RenderItem::LineOnly;

		auto psoConfig = _psoConfig;
		// Only masking relevant flags
		psoConfig.PsoFlags &= relevantFlags;

		GraphicsPSO depthPSO = GraphicsPSO();

		std::wstring name = L"DepthPSO";

		depthPSO.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		depthPSO.SetDepthStencilState(DepthStateReadWrite);
		depthPSO.SetBlendState(BlendDisable);
		depthPSO.SetSampleMask(UINT_MAX);

		// Handling VS and PS shader
		if ((psoConfig.PSIndex | psoConfig.VSIndex) == 0)
		{
			// Alpha testing
			if (psoConfig.PsoFlags & RenderItem::AlphaTest)
			{
				name += L" Cutout";

				depthPSO.SetPixelShader(ShaderData("CutoutDepthPS"));

				// Has skin
				if (psoConfig.PsoFlags & RenderItem::HasSkin)
				{
					name += L" Skinned";
					depthPSO.SetVertexShader(ShaderData("CutoutDepthSkinVS"));
				}
				else // Doesn't have skin
				{
					depthPSO.SetVertexShader(ShaderData("CutoutDepthVS"));
				}
			}
			else // No alpha testing
			{
				// Has skin
				if (psoConfig.PsoFlags & RenderItem::HasSkin)
				{
					name += L" Skinned";
					depthPSO.SetVertexShader(ShaderData("DepthOnlySkinVS"));
				}
				else // Doesn't have skin
				{
					depthPSO.SetVertexShader(ShaderData("DepthOnlyVS"));
				}
			}
		}
		else
		{
			auto vShader = GetShaderByIndex(psoConfig.VSIndex);
			auto pShader = GetShaderByIndex(psoConfig.PSIndex);

			if (vShader)
				depthPSO.SetVertexShader(vShader->GetBufferPointer(), vShader->GetBufferSize());

			if (pShader)
				depthPSO.SetPixelShader(pShader->GetBufferPointer(), pShader->GetBufferSize());
		}

		// Setting Geometry Shader
		if (psoConfig.GSIndex > 0)
		{
			auto gShader = GetShaderByIndex(psoConfig.GSIndex);
			depthPSO.SetGeometryShader(gShader->GetBufferPointer(), gShader->GetBufferSize());
		}

		if (psoConfig.HSIndex > 0)
		{
			auto gShader = GetShaderByIndex(psoConfig.HSIndex);
			depthPSO.SetHullShader(gShader->GetBufferPointer(), gShader->GetBufferSize());
		}

		if (psoConfig.DSIndex > 0)
		{
			auto gShader = GetShaderByIndex(psoConfig.DSIndex);
			depthPSO.SetDomainShader(gShader->GetBufferPointer(), gShader->GetBufferSize());
		}

		// Handling Rasterizer

		// Is two sided
		if (psoConfig.PsoFlags & RenderItem::TwoSided)
		{
			name += L" TwoSided";
			// Is wireframed
			if (psoConfig.PsoFlags & RenderItem::Wireframe)
			{
				name += L" Wireframe";
				depthPSO.SetRasterizerState(RasterizerTwoSidedWireframe);
			}
			else // Not wireframed
			{
				depthPSO.SetRasterizerState(RasterizerTwoSided);
			}
		}
		else // Not two sided
		{
			// Is wireframed
			if (psoConfig.PsoFlags & RenderItem::Wireframe)
			{
				name += L" Wireframe";
				depthPSO.SetRasterizerState(RasterizerDefaultWireframe);
			}
			else // Not wireframed
			{
				depthPSO.SetRasterizerState(RasterizerDefault);
			}
		}

		// Handling input layout
		if ((psoConfig.InputLayout.NumElements == 0) && (psoConfig.PsoFlags & RenderItem::SkipVertexIndex) == 0)
		{
			if (psoConfig.PsoFlags & RenderItem::HasSkin)
			{
				depthPSO.SetInputLayout(VertexData(D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned));
			}
			else
			{
				depthPSO.SetInputLayout(VertexData(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture));
			}
		}
		else
		{
			depthPSO.SetInputLayout(psoConfig.InputLayout.NumElements, psoConfig.InputLayout.pInputElementDescs);
		}

		// Setting primitive topology (line / triangle)
		if (psoConfig.PsoFlags & RenderItem::LineOnly && psoConfig.PsoFlags & RenderItem::PointOnly)
		{
			name += L" Patch";
			depthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);
		}
		else if (psoConfig.PsoFlags & RenderItem::LineOnly)
		{
			name += L" Line";
			depthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		}
		else if (psoConfig.PsoFlags & RenderItem::PointOnly)
		{
			name += L" Point";
			depthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
		}
		else
		{
			depthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		}

		// Setting render target format
		depthPSO.SetRenderTargetFormats(0, nullptr, D_GRAPHICS::GetDepthFormat());

		depthPSO.Finalize(name);


		// Look for an existing PSO
		for (uint32_t i = 0; i < Psos.size(); ++i)
		{
			if (depthPSO.GetPipelineStateObject() == Psos[i].GetPipelineStateObject())
			{
				return i;
			}
		}

		Psos.push_back(depthPSO);

		// Setting up shadow one just one index ahead

		depthPSO.SetRenderTargetFormats(0, nullptr, D_GRAPHICS::GetShadowFormat());

		// Handling Shadow Rasterizer
		// Is two sided
		if (psoConfig.PsoFlags & RenderItem::TwoSided)
		{
			// Is wireframed
			if (psoConfig.PsoFlags & RenderItem::Wireframe)
			{
				depthPSO.SetRasterizerState(RasterizerShadowTwoSidedWireframe);
			}
			else // Not wireframed
			{
				depthPSO.SetRasterizerState(RasterizerShadowTwoSided);
			}
		}
		else // Not two sided
		{
			// Is wireframed
			if (psoConfig.PsoFlags & RenderItem::Wireframe)
			{
				depthPSO.SetRasterizerState(RasterizerShadowWireframe);
			}
			else // Not wireframed
			{
				depthPSO.SetRasterizerState(RasterizerShadow);
			}
		}

		name += L" Shadow";
		depthPSO.Finalize(name);
		Psos.push_back(depthPSO);

		auto index = (UINT)Psos.size() - 2u;

		return index;
	}

	UINT GetRenderPso(PsoConfig const& psoConfig)
	{
		GraphicsPSO ColorPSO = DefaultPso;
		uint16_t Requirements = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0;

		// Checking requirements and supported features
		// Before removing any of unsupported check asserts, make sure the appropriate shader exists
		// and is binded to a pipeline
		D_ASSERT((psoConfig.PsoFlags & Requirements) == Requirements);
		D_ASSERT_M(!(psoConfig.PsoFlags & RenderItem::HasUV1), "Higher level UV sets are not supported yet");

		std::wstring psoName;

		// Setting input layout
		if (psoConfig.InputLayout.NumElements == 0 && (psoConfig.PsoFlags & RenderItem::SkipVertexIndex) == 0)
		{
			if (psoConfig.PsoFlags & RenderItem::HasSkin)
			{
				ColorPSO.SetInputLayout(VertexData(D_RENDERER_VERTEX::VertexPositionNormalTangentTextureSkinned));
			}
			else
				ColorPSO.SetInputLayout(VertexData(D_RENDERER_VERTEX::VertexPositionNormalTangentTexture));
		}

#define GET_SHADER(name) Psos[name]

		if ((psoConfig.PSIndex | psoConfig.VSIndex) == 0)
		{
			if (psoConfig.PsoFlags & RenderItem::ColorOnly)
			{
				ColorPSO.SetVertexShader(ShaderData("ColorVS"));
				ColorPSO.SetPixelShader(ShaderData("ColorPS"));

				psoName += L"Color Only ";
			}
			else
			{
				if (psoConfig.PsoFlags & RenderItem::HasSkin)
				{
					if (psoConfig.PsoFlags & RenderItem::HasTangent)
					{
						if (psoConfig.PsoFlags & RenderItem::HasUV1)
						{
							// TODO: Change to a shader supporting UV1
							D_ASSERT_M(true, "UV1 is currently not supported");
						}
						else
						{
							ColorPSO.SetVertexShader(ShaderData("SkinnedVS"));
							ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
						}
					}
					else
					{
						// TODO: Change all these to a shader without tangent parameter for every vertext
						if (psoConfig.PsoFlags & RenderItem::HasUV1)
						{
							D_ASSERT_M(true, "UV1 is currently not supported");
						}
						else
						{
							ColorPSO.SetVertexShader(ShaderData("SkinnedVS"));
							ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
						}
					}
					psoName += L"Skinned ";
				}
				else
				{
					if (psoConfig.PsoFlags & RenderItem::HasTangent)
					{
						if (psoConfig.PsoFlags & RenderItem::HasUV1)
						{
							// TODO: Change to a shader supporting UV1
							D_ASSERT_M(true, "UV1 is currently not supported");
						}
						else
						{
							ColorPSO.SetVertexShader(ShaderData("DefaultVS"));
							ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
						}
					}
					else
					{
						// TODO: Change all these to a shader without tangent parameter for every vertext
						if (psoConfig.PsoFlags & RenderItem::HasUV1)
						{
							D_ASSERT_M(true, "Higher level UV sets are not supported yet");
						}
						else
						{
							ColorPSO.SetVertexShader(ShaderData("DefaultVS"));
							ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
						}
					}
				}
			}
		}
		else
		{
			auto vShader = GetShaderByIndex(psoConfig.VSIndex);
			auto pShader = GetShaderByIndex(psoConfig.PSIndex);

			if (vShader)
				ColorPSO.SetVertexShader(vShader->GetBufferPointer(), vShader->GetBufferSize());

			if (pShader)
				ColorPSO.SetPixelShader(pShader->GetBufferPointer(), pShader->GetBufferSize());
		}

		// Setting Geometry Shader
		if (psoConfig.GSIndex > 0)
		{
			auto gShader = GetShaderByIndex(psoConfig.GSIndex);
			ColorPSO.SetGeometryShader(gShader->GetBufferPointer(), gShader->GetBufferSize());

			psoName += L"Geometry:" + std::to_wstring(psoConfig.GSIndex);
		}

		if (psoConfig.HSIndex > 0)
		{
			auto gShader = GetShaderByIndex(psoConfig.HSIndex);
			ColorPSO.SetHullShader(gShader->GetBufferPointer(), gShader->GetBufferSize());

			psoName += L"Hull:" + std::to_wstring(psoConfig.HSIndex);
		}

		if (psoConfig.DSIndex > 0)
		{
			auto gShader = GetShaderByIndex(psoConfig.DSIndex);
			ColorPSO.SetDomainShader(gShader->GetBufferPointer(), gShader->GetBufferSize());

			psoName += L"Domain:" + std::to_wstring(psoConfig.DSIndex);
		}

		if (psoConfig.PsoFlags & RenderItem::AlphaBlend)
		{
			ColorPSO.SetBlendState(BlendTraditional);
			ColorPSO.SetDepthStencilState(DepthStateReadOnly);
			psoName += L"AlphaBlended ";
		}
		if (psoConfig.PsoFlags & RenderItem::TwoSided)
		{
			if (psoConfig.PsoFlags & RenderItem::Wireframe)
			{
				ColorPSO.SetRasterizerState(RasterizerTwoSidedWireframe);
				psoName += L"Wireframed ";
			}
			else
				ColorPSO.SetRasterizerState(RasterizerTwoSided);

			psoName += L"TwoSided ";
		}
		else
		{
			if (psoConfig.PsoFlags & RenderItem::Wireframe)
			{
				ColorPSO.SetRasterizerState(RasterizerDefaultWireframe);
				psoName += L"Wireframed ";
			}
			else
				ColorPSO.SetRasterizerState(RasterizerDefault);
		}

		// Setting primitive topology (line / triangle)
		if (psoConfig.PsoFlags & RenderItem::LineOnly && psoConfig.PsoFlags & RenderItem::PointOnly)
		{
			ColorPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH);
			psoName += L" Patch";
		}
		else if (psoConfig.PsoFlags & RenderItem::LineOnly)
		{
			ColorPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
			psoName += L"Line ";
		}
		else if (psoConfig.PsoFlags & RenderItem::PointOnly)
		{
			ColorPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT);
			psoName += L"Point ";
		}
		else
		{
			ColorPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
			psoName += L"Triangle ";
		}
		ColorPSO.Finalize(psoName);

		// Look for an existing PSO
		for (uint32_t i = 0; i < Psos.size(); ++i)
		{
			if (ColorPSO.GetPipelineStateObject() == Psos[i].GetPipelineStateObject())
			{
				return i;
			}
		}

		// If not found, keep the new one, and return its index
		Psos.push_back(ColorPSO);

		// The returned PSO index has read-write depth.  The index+1 tests for equal depth.
		ColorPSO.SetDepthStencilState(DepthStateTestEqual);
		ColorPSO.Finalize();
#ifdef _DEBUG
		for (uint32_t i = 0; i < Psos.size(); ++i)
			D_ASSERT(ColorPSO.GetPipelineStateObject() != Psos[i].GetPipelineStateObject());
#endif
		Psos.push_back(ColorPSO);

		D_ASSERT_M(Psos.size() <= UINT_MAX, "Ran out of room for unique PSOs");

		auto index = (UINT)Psos.size() - 2;
		return index;
	}

	UINT GetPso(PsoConfig const& psoConfig)
	{
		if (D_RENDERER::GetActiveRendererType() != D_RENDERER::RendererType::Rasterization)
			return UINT_MAX;

		if (psoConfig.PsoFlags & RenderItem::DepthOnly)
			return GetDepthOnlyPso(psoConfig);
		else
			return GetRenderPso(psoConfig);
	}

	D_CONTAINERS::DVector<D_GRAPHICS_UTILS::GraphicsPSO> const& GetPsos()
	{
		return Psos;
	}

	DescriptorHandle AllocateTextureDescriptor(UINT count)
	{
		return TextureHeap.Alloc(count);
	}

	DescriptorHandle AllocateSamplerDescriptor(UINT count)
	{
		return SamplerHeap.Alloc(count);
	}

}