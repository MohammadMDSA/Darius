#include "Renderer.hpp"
#include "pch.hpp"

#include "AntiAliasing/TemporalEffect.hpp"
#include "AmbientOcclusion/SuperSampleAmbientOcclusion.hpp"
#include "Camera/CameraManager.hpp"
#include "Components/MeshRendererComponent.hpp"
#include "Components/SkeletalMeshRendererComponent.hpp"
#include "Geometry/Mesh.hpp"
#include "FrameResource.hpp"
#include "GraphicsCore.hpp"
#include "GraphicsUtils/D3DUtils.hpp"
#include "GraphicsUtils/RootSignature.hpp"
#include "GraphicsUtils/Memory/DescriptorHeap.hpp"
#include "GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "GraphicsUtils/VertexTypes.hpp"
#include "GraphicsUtils/Profiling/Profiling.hpp"
#include "Light/LightManager.hpp"
#include "PostProcessing/MotionBlur.hpp"
#include "PostProcessing/PostProcessing.hpp"
#include "Resources/TextureResource.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Job/Job.hpp>
#include <Math/VectorMath.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Utils/Assert.hpp>

#include <imgui.h>
#include <implot.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include <filesystem>

using namespace D_CONTAINERS;
using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_GRAPHICS;
using namespace D_GRAPHICS_MEMORY;
using namespace D_GRAPHICS_UTILS;
using namespace D_RENDERER_FRAME_RESOURCE;
using namespace D_RENDERER_GEOMETRY;
using namespace D_RESOURCE;
using namespace Microsoft::WRL;

#define VertexData(il) il::InputLayout.NumElements, il::InputLayout.pInputElementDescs
#define ShaderData(name) Shaders[name]->GetBufferPointer(), Shaders[name]->GetBufferSize()

namespace Darius::Renderer
{
	bool												_initialized = false;

#ifdef _D_EDITOR
	DescriptorHeap										ImguiHeap;
#endif

	// Input layout and root signature
	std::array<D_GRAPHICS_UTILS::RootSignature, (size_t)RootSignatureTypes::_numRootSig> RootSigns;
	DVector<D_GRAPHICS_UTILS::GraphicsPSO> Psos;

	DescriptorHeap										TextureHeap;
	DescriptorHeap										SamplerHeap;

	DescriptorHandle									CommonTexture[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
	DescriptorHandle									CommonTextureSamplers;

	D_CORE::Ref<TextureResource>						RadianceCubeMap;
	D_CORE::Ref<TextureResource>						IrradianceCubeMap;
	D_CORE::Ref<TextureResource>						DefaultBlackCubeMap;
	float												SpecularIBLRange;
	float												SpecularIBLBias = FLT_MAX;

	GraphicsPSO											DefaultPso;
	GraphicsPSO											SkyboxPso;

	//////////////////////////////////////////////////////
	// Options
	bool												SeparateZPass = true;

	//////////////////////////////////////////////////////
	// Functions
#ifdef _D_EDITOR
	void InitializeGUI();
#endif
	void BuildPSO();
	void BuildRootSignature();

	void Clear(D_GRAPHICS::GraphicsContext& context, D_GRAPHICS_BUFFERS::ColorBuffer& rt, D_GRAPHICS_BUFFERS::DepthBuffer& depthStencil, RECT bounds, std::wstring const& processName = L"Clear");


	void Initialize(D_SERIALIZATION::Json const& settings)
	{

		D_ASSERT(!_initialized);

		D_H_OPTIONS_LOAD_BASIC("Passes.SeparateZ", SeparateZPass);

		BuildRootSignature();
		BuildPSO();

		TextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
		SamplerHeap.Create(L"Scene Sampler Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);

		for (int i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			CommonTexture[i] = TextureHeap.Alloc(8);
		}
		CommonTextureSamplers = SamplerHeap.Alloc(kNumTextures);

#ifdef _D_EDITOR
		InitializeGUI();
#endif // _D_EDITOR
		DefaultBlackCubeMap = D_RESOURCE::GetResource<TextureResource>(GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::TextureCubeMapBlack), nullptr, L"Renderer", "Engine Subsystem");

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

			for (int i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
			{
				DescriptorHandle dest = CommonTexture[i] + 6 * TextureHeap.GetDescriptorSize();

				D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &dest, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}
		}

		// Initializing Camera Manager
		D_CAMERA_MANAGER::Initialize();
	}

	void Shutdown()
	{
		D_CAMERA_MANAGER::Shutdown();

#ifdef _D_EDITOR
		ImguiHeap.Destroy();
#endif
		TextureHeap.Destroy();
		SamplerHeap.Destroy();
	}

	void Update()
	{

		D_CAMERA_MANAGER::Update();

		auto& reg = D_WORLD::GetRegistry();

		reg.each([&](D_GRAPHICS::MeshRendererComponent& meshComp)
			{
				D_JOB::AssignTask([&](int threadNumber, int)
					{
						meshComp.Update(-1);

					});
			}
		);

		reg.each([&](D_GRAPHICS::SkeletalMeshRendererComponent& meshComp)
			{
				D_JOB::AssignTask([&](int threadNumber, int)
					{
						meshComp.Update(-1);

					});
			}
		);

		if (D_JOB::IsMainThread())
			Darius::Job::WaitForThreadsToFinish();
		{
			D_PROFILING::ScopedTimer lightProfiling(L"Update Lights");
			::D_LIGHT::Update();
		}
	}

	void AddRenderItems(D_RENDERER::MeshSorter& sorter, D_MATH_CAMERA::BaseCamera const& cam)
	{
		auto& worldReg = D_WORLD::GetRegistry();

		auto frustum = cam.GetViewSpaceFrustum();

		// Iterating over meshes
		worldReg.each([&](D_GRAPHICS::MeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				// Is it in our frustum
				auto sphereWorldSpace = meshComp.GetGameObject()->GetTransform() * meshComp.GetBounds();
				auto sphereViewSpace = BoundingSphere(Vector3(cam.GetViewMatrix() * sphereWorldSpace.GetCenter()), sphereWorldSpace.GetRadius());
				if (!frustum.IntersectSphere(sphereViewSpace))
					return;

				auto distance = -sphereViewSpace.GetCenter().GetZ() - sphereViewSpace.GetRadius();
				sorter.AddMesh(meshComp.GetRenderItem(), distance);
			});

		// Iterating over meshes
		worldReg.each([&](D_GRAPHICS::SkeletalMeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				// Is it in our frustum
				auto sphereWorldSpace = meshComp.GetGameObject()->GetTransform() * meshComp.GetBounds();
				auto sphereViewSpace = BoundingSphere(Vector3(cam.GetViewMatrix() * sphereWorldSpace.GetCenter()), sphereWorldSpace.GetRadius());
				if (!frustum.IntersectSphere(sphereViewSpace))
					return;

				auto distance = -sphereViewSpace.GetCenter().GetZ() - sphereViewSpace.GetRadius();
				sorter.AddMesh(meshComp.GetRenderItem(), distance);
			});

		//D_LOG_DEBUG("Number of render items: " << sorter.CountObjects());

	}

	void AddShadowRenderItems(D_CONTAINERS::DVector<RenderItem>& items)
	{
		auto& worldReg = D_WORLD::GetRegistry();

		// Iterating over meshes
		worldReg.each([&](D_GRAPHICS::MeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				if (!meshComp.IsCastsShadow())
					return;

				items.push_back(meshComp.GetRenderItem());
			});

		// Iterating over meshes
		worldReg.each([&](D_GRAPHICS::SkeletalMeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				if (!meshComp.IsCastsShadow())
					return;

				items.push_back(meshComp.GetRenderItem());
			});
	}

	void Render(std::wstring const& jobId, SceneRenderContext& rContext, std::function<void(MeshSorter&)> additionalMainDraw, std::function<void(MeshSorter&)> postDraw)
	{
		auto& context = rContext.GraphicsContext;

		// Clearing depth and scene color textures
		context.TransitionResource(rContext.DepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		context.TransitionResource(rContext.ColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET);
		context.TransitionResource(rContext.NormalBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		context.ClearDepth(rContext.DepthBuffer);
		context.ClearColor(rContext.ColorBuffer);
		context.ClearColor(rContext.NormalBuffer);

		auto width = rContext.ColorBuffer.GetWidth();
		auto height = rContext.ColorBuffer.GetHeight();

		// Setting up sorter
		auto viewPort = CD3DX12_VIEWPORT(0.f, 0.f, (float)width, (float)height, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissor = CD3DX12_RECT(0l, 0l, (long)width, (long)height);
		MeshSorter sorter(MeshSorter::kDefault);
		sorter.SetCamera(rContext.Camera);
		sorter.SetViewport(viewPort);
		sorter.SetScissor(scissor);
		sorter.SetDepthStencilTarget(rContext.DepthBuffer);
		sorter.AddRenderTarget(rContext.ColorBuffer);
		sorter.SetNormalTarget(rContext.NormalBuffer);

		// Add meshes to sorter
		AddRenderItems(sorter, rContext.Camera);

		{
			// Creating shadows

			DVector<RenderItem> shadowRenderItems;
			AddShadowRenderItems(shadowRenderItems);

			D_LIGHT::RenderShadows(rContext.Camera, shadowRenderItems, context);
		}

		if (additionalMainDraw)
			additionalMainDraw(sorter);

		sorter.Sort();

		if (rContext.DrawSkybox)
			D_RENDERER::DrawSkybox(context, rContext.Camera, rContext.ColorBuffer, rContext.DepthBuffer, viewPort, scissor);

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

		if (postDraw)
		{
			MeshSorter additionalSorter(sorter);

			postDraw(additionalSorter);
		}

		auto frameIdxMod2 = D_GRAPHICS_AA_TEMPORAL::GetFrameIndexMod2();
		auto& commandContext = context.GetComputeContext();

		D_GRAPHICS_PP_MOTION::MotionBlurBuffers motionBuffers = { rContext.ColorBuffer, rContext.LinearDepth[frameIdxMod2], rContext.VelocityBuffer, rContext.DepthBuffer };

		D_GRAPHICS_PP_MOTION::GenerateCameraVelocityBuffer(commandContext, motionBuffers, rContext.Camera);

		D_GRAPHICS_AA_TEMPORAL::ResolveImage(commandContext, rContext.ColorBuffer, rContext.VelocityBuffer, rContext.TemporalColor, rContext.LinearDepth);
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		D_H_OPTION_DRAW_CHECKBOX("Separate Z Pass", "Passes.SeparateZ", SeparateZPass);

		D_H_OPTION_DRAW_END();
	}
#endif

	D_GRAPHICS_UTILS::RootSignature& GetRootSignature(RootSignatureTypes type)
	{
		return RootSigns[(size_t)type];
	}

	void Present()
	{
		D_GRAPHICS::Present();
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

	void BuildPSO()
	{

		DXGI_FORMAT ColorFormat = D_GRAPHICS::GetColorFormat();
		DXGI_FORMAT DepthFormat = D_GRAPHICS::GetDepthFormat();
		DXGI_FORMAT ShadowFormat = D_GRAPHICS::GetShadowFormat();

		// For Opaque objects
		DefaultPso = GraphicsPSO(L"Opaque PSO");
		DefaultPso.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture));
		DefaultPso.SetVertexShader(ShaderData("DefaultVS"));
		DefaultPso.SetPixelShader(ShaderData("DefaultPS"));
		DefaultPso.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		DefaultPso.SetRasterizerState(D_GRAPHICS::RasterizerDefault);
		DefaultPso.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		DefaultPso.SetDepthStencilState(DepthStateReadWrite);
		DefaultPso.SetSampleMask(UINT_MAX);
		DefaultPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		DefaultPso.SetRenderTargetFormat(ColorFormat, DepthFormat);
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
		D_ASSERT(Psos.size() == 0);

		// Depth Only PSOs

		GraphicsPSO DepthOnlyPSO(L"Renderer: Depth Only PSO");
		DepthOnlyPSO.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		DepthOnlyPSO.SetRasterizerState(RasterizerDefault);
		DepthOnlyPSO.SetBlendState(BlendDisable);
		DepthOnlyPSO.SetDepthStencilState(DepthStateReadWrite);
		DepthOnlyPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture));
		DepthOnlyPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		DepthOnlyPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
		DepthOnlyPSO.SetVertexShader(ShaderData("DepthOnlyVS"));
		DepthOnlyPSO.Finalize();
		Psos.push_back(DepthOnlyPSO);

		GraphicsPSO CutoutDepthPSO(L"Renderer: Cutout Depth PSO");
		CutoutDepthPSO = DepthOnlyPSO;
		CutoutDepthPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture));
		CutoutDepthPSO.SetRasterizerState(RasterizerDefault);
		CutoutDepthPSO.SetVertexShader(ShaderData("CutoutDepthVS"));
		CutoutDepthPSO.SetPixelShader(ShaderData("CutoutDepthPS"));
		CutoutDepthPSO.Finalize();
		Psos.push_back(CutoutDepthPSO);

		GraphicsPSO SkinDepthOnlyPSO = DepthOnlyPSO;
		SkinDepthOnlyPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned));
		SkinDepthOnlyPSO.SetVertexShader(ShaderData("DepthOnlySkinVS"));
		SkinDepthOnlyPSO.Finalize();
		Psos.push_back(SkinDepthOnlyPSO);

		GraphicsPSO SkinCutoutDepthPSO = CutoutDepthPSO;
		SkinCutoutDepthPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned));
		SkinCutoutDepthPSO.SetVertexShader(ShaderData("CutoutDepthSkinVS"));
		SkinCutoutDepthPSO.Finalize();
		Psos.push_back(SkinCutoutDepthPSO);

		// Depth Only Two Sided PSOs

		GraphicsPSO DepthOnlyTwoSidedPSO(L"Renderer: Depth Only PSO Two Sided");
		DepthOnlyTwoSidedPSO.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		DepthOnlyTwoSidedPSO.SetRasterizerState(RasterizerTwoSided);
		DepthOnlyTwoSidedPSO.SetBlendState(BlendDisable);
		DepthOnlyTwoSidedPSO.SetDepthStencilState(DepthStateReadWrite);
		DepthOnlyTwoSidedPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture));
		DepthOnlyTwoSidedPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		DepthOnlyTwoSidedPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
		DepthOnlyTwoSidedPSO.SetVertexShader(ShaderData("DepthOnlyVS"));
		DepthOnlyTwoSidedPSO.Finalize();
		Psos.push_back(DepthOnlyTwoSidedPSO);

		GraphicsPSO CutoutDepthTwoSidedPSO(L"Renderer: Cutout Depth PSO Two Sided");
		CutoutDepthTwoSidedPSO = DepthOnlyTwoSidedPSO;
		CutoutDepthTwoSidedPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture));
		CutoutDepthTwoSidedPSO.SetRasterizerState(RasterizerTwoSided);
		CutoutDepthTwoSidedPSO.SetVertexShader(ShaderData("CutoutDepthVS"));
		CutoutDepthTwoSidedPSO.SetPixelShader(ShaderData("CutoutDepthPS"));
		CutoutDepthTwoSidedPSO.Finalize();
		Psos.push_back(CutoutDepthTwoSidedPSO);

		GraphicsPSO SkinDepthOnlyTwoSidedPSO = DepthOnlyTwoSidedPSO;
		SkinDepthOnlyTwoSidedPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned));
		SkinDepthOnlyTwoSidedPSO.SetVertexShader(ShaderData("DepthOnlySkinVS"));
		SkinDepthOnlyTwoSidedPSO.Finalize();
		Psos.push_back(SkinDepthOnlyTwoSidedPSO);

		GraphicsPSO SkinCutoutDepthTwoSidedPSO = CutoutDepthTwoSidedPSO;
		SkinCutoutDepthTwoSidedPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned));
		SkinCutoutDepthTwoSidedPSO.SetVertexShader(ShaderData("CutoutDepthSkinVS"));
		SkinCutoutDepthTwoSidedPSO.Finalize();
		Psos.push_back(SkinCutoutDepthTwoSidedPSO);


		// Depth Only Wireframe PSOs

		GraphicsPSO DepthOnlyWireframePSO(L"Renderer: Depth Only PSO Wireframe");
		DepthOnlyWireframePSO = DepthOnlyPSO;
		DepthOnlyWireframePSO.SetRasterizerState(RasterizerDefaultWireframe);
		DepthOnlyWireframePSO.Finalize();
		Psos.push_back(DepthOnlyWireframePSO);

		GraphicsPSO CutoutDepthWireframePSO(L"Renderer: Cutout Depth PSO Wireframe");
		CutoutDepthWireframePSO = CutoutDepthPSO;
		CutoutDepthWireframePSO.SetRasterizerState(RasterizerDefaultWireframe);
		CutoutDepthWireframePSO.Finalize();
		Psos.push_back(CutoutDepthWireframePSO);

		GraphicsPSO SkinDepthOnlyWireframePSO(L"Renderer: Depth Only PSO Skinned Wireframe");
		SkinDepthOnlyWireframePSO = SkinDepthOnlyPSO;
		SkinDepthOnlyWireframePSO.SetRasterizerState(RasterizerDefaultWireframe);
		SkinDepthOnlyWireframePSO.Finalize();
		Psos.push_back(SkinDepthOnlyWireframePSO);

		GraphicsPSO SkinCutoutDepthWireframePSO(L"Renderer: Cutout Depth PSO Skinned Wireframe");
		SkinCutoutDepthWireframePSO = SkinCutoutDepthPSO;
		SkinCutoutDepthWireframePSO.SetRasterizerState(RasterizerDefaultWireframe);
		SkinCutoutDepthWireframePSO.Finalize();
		Psos.push_back(SkinCutoutDepthWireframePSO);

		// Depth Only Two Sided Wireframe PSOs
		GraphicsPSO DepthOnlyTwoSidedWireframePSO(L"Renderer: Depth Only PSO Two Sided Wireframe");
		DepthOnlyTwoSidedWireframePSO = DepthOnlyTwoSidedPSO;
		DepthOnlyTwoSidedWireframePSO.SetRasterizerState(RasterizerTwoSidedWireframe);
		DepthOnlyTwoSidedWireframePSO.Finalize();
		Psos.push_back(DepthOnlyTwoSidedWireframePSO);


		GraphicsPSO CutoutDepthTwoSidedWireframePSO(L"Renderer: Cutout Depth PSO Two Sided Wireframe");
		CutoutDepthTwoSidedWireframePSO = CutoutDepthTwoSidedPSO;
		CutoutDepthTwoSidedWireframePSO.SetRasterizerState(RasterizerTwoSidedWireframe);
		CutoutDepthTwoSidedWireframePSO.Finalize();
		Psos.push_back(CutoutDepthTwoSidedWireframePSO);


		GraphicsPSO SkinDepthOnlyTwoSidedWireframePSO(L"Renderer: Depth Only PSO Skinned Two Sided Wireframe");
		SkinDepthOnlyTwoSidedWireframePSO = SkinDepthOnlyTwoSidedPSO;
		SkinDepthOnlyTwoSidedWireframePSO.SetRasterizerState(RasterizerTwoSidedWireframe);
		SkinDepthOnlyTwoSidedWireframePSO.Finalize();
		Psos.push_back(SkinDepthOnlyTwoSidedWireframePSO);


		GraphicsPSO SkinCutoutDepthTwoSidedWireframePSO(L"Renderer: Cutout Depth PSO Skinned Two Sided Wireframe");
		SkinCutoutDepthTwoSidedWireframePSO = SkinCutoutDepthTwoSidedPSO;
		SkinCutoutDepthTwoSidedWireframePSO.SetRasterizerState(RasterizerTwoSidedWireframe);
		SkinCutoutDepthTwoSidedWireframePSO.Finalize();
		Psos.push_back(SkinCutoutDepthTwoSidedWireframePSO);



		D_ASSERT(Psos.size() == 16);

		// Shadow PSOs

		DepthOnlyPSO.SetRasterizerState(RasterizerShadow);
		DepthOnlyPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		DepthOnlyPSO.Finalize();
		Psos.push_back(DepthOnlyPSO);

		CutoutDepthPSO.SetRasterizerState(RasterizerShadowTwoSided);
		CutoutDepthPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		CutoutDepthPSO.Finalize();
		Psos.push_back(CutoutDepthPSO);

		SkinDepthOnlyPSO.SetRasterizerState(RasterizerShadow);
		SkinDepthOnlyPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		SkinDepthOnlyPSO.Finalize();
		Psos.push_back(SkinDepthOnlyPSO);

		SkinCutoutDepthPSO.SetRasterizerState(RasterizerShadowTwoSided);
		SkinCutoutDepthPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		SkinCutoutDepthPSO.Finalize();
		Psos.push_back(SkinCutoutDepthPSO);

		// Shadow Two Sided PSOs

		DepthOnlyTwoSidedPSO.SetRasterizerState(RasterizerShadow);
		DepthOnlyTwoSidedPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		DepthOnlyTwoSidedPSO.Finalize();
		Psos.push_back(DepthOnlyTwoSidedPSO);

		CutoutDepthTwoSidedPSO.SetRasterizerState(RasterizerShadowTwoSided);
		CutoutDepthTwoSidedPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		CutoutDepthTwoSidedPSO.Finalize();
		Psos.push_back(CutoutDepthTwoSidedPSO);

		SkinDepthOnlyTwoSidedPSO.SetRasterizerState(RasterizerShadow);
		SkinDepthOnlyTwoSidedPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		SkinDepthOnlyTwoSidedPSO.Finalize();
		Psos.push_back(SkinDepthOnlyTwoSidedPSO);

		SkinCutoutDepthTwoSidedPSO.SetRasterizerState(RasterizerShadowTwoSided);
		SkinCutoutDepthTwoSidedPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		SkinCutoutDepthTwoSidedPSO.Finalize();
		Psos.push_back(SkinCutoutDepthTwoSidedPSO);
	}

	void BuildRootSignature()
	{
		// Default root signature
		auto& def = RootSigns[(size_t)RootSignatureTypes::DefaultRootSig];
		def.Reset(kNumRootBindings, 4);

		// Create samplers
		SamplerDesc defaultSamplerDesc;
		defaultSamplerDesc.MaxAnisotropy = 8;
		SamplerDesc cubeMapSamplerDesc = defaultSamplerDesc;
		def.InitStaticSampler(10, defaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(11, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(12, cubeMapSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(13, SamplerLinearWrapDesc, D3D12_SHADER_VISIBILITY_PIXEL);

		// Create root CBVs.
		def[kMeshConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
		def[kMaterialConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kMaterialSRVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kMaterialSamplers].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 10, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kCommonCBV].InitAsConstantBuffer(1);
		def[kCommonSRVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 8, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kSkinMatrices].InitAsBufferSRV(20, D3D12_SHADER_VISIBILITY_VERTEX);
		def.Finalize(L"Main Root Sig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}

	DescriptorHandle AllocateTextureDescriptor(UINT count)
	{
		return TextureHeap.Alloc(count);
	}

	void SetIBLTextures(D_CORE::Ref<TextureResource>& diffuseIBL, D_CORE::Ref<TextureResource>& specularIBL)
	{
		RadianceCubeMap = specularIBL;
		IrradianceCubeMap = diffuseIBL;

		SpecularIBLRange = 0.f;
		if (RadianceCubeMap.IsValid())
		{
			auto texRes = const_cast<ID3D12Resource*>(RadianceCubeMap->GetTextureData()->GetResource());
			const D3D12_RESOURCE_DESC& texDesc = texRes->GetDesc();
			SpecularIBLRange = D_MATH::Max(0.f, (float)texDesc.MipLevels - 1);
			SpecularIBLBias = D_MATH::Min(SpecularIBLBias, SpecularIBLRange);
		}

		uint32_t DestCount = 2;
		uint32_t SourceCounts[] = { 1, 1 };

		D3D12_CPU_DESCRIPTOR_HANDLE specHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE diffHandle;
		if (specularIBL.IsValid())
			specHandle = specularIBL->GetTextureData()->GetSRV();
		else
			specHandle = DefaultBlackCubeMap->GetTextureData()->GetSRV();

		if (diffuseIBL.IsValid())
			diffHandle = diffuseIBL->GetTextureData()->GetSRV();
		else
			diffHandle = DefaultBlackCubeMap->GetTextureData()->GetSRV();

		D3D12_CPU_DESCRIPTOR_HANDLE SourceTextures[] =
		{
			specHandle,
			diffHandle
		};

		for (int i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			DescriptorHandle dest = CommonTexture[i] + 6 * TextureHeap.GetDescriptorSize();

			D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &dest, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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
		context.SetDynamicConstantBufferView(kMeshConstants, sizeof(SkyboxVSCB), &skyVSCB);
		context.SetDynamicConstantBufferView(kMaterialConstants, sizeof(SkyboxPSCB), &skyPSVB);
		context.SetDescriptorTable(kCommonSRVs, CommonTexture[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()]);

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
		bool skinned = (renderItem.PsoFlags & RenderItem::HasSkin) == RenderItem::HasSkin;
		bool twoSided = renderItem.PsoFlags & RenderItem::TwoSided;
		bool wireframed = renderItem.PsoFlags & RenderItem::Wireframe;
		uint64_t depthPSO = (skinned ? 2 : 0) + (alphaTest ? 1 : 0) + (twoSided ? 4 : 0) + (wireframed ? 8 : 0);
		uint64_t shadowDepthPSO = (skinned ? 2 : 0) + (alphaTest ? 1 : 0) + (twoSided ? 4 : 0);

		union float_or_int { float f; uint32_t u; } dist;
		dist.f = Max(distance, 0.0f);

		if (m_BatchType == kShadows)
		{
			if (alphaBlend)
				return;

			key.passID = kZPass;
			key.psoIdx = shadowDepthPSO + 16;
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
		context.SetDescriptorTable(kCommonSRVs, CommonTexture[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()]);

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

				m_Scissor.left = 1;
				m_Scissor.right = m_DSV->GetWidth() - 2;
				m_Scissor.top = 1;
				m_Scissor.bottom = m_DSV->GetHeight() - 2;
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
					D_LIGHT::GetLightMaskHandle(),
					D_LIGHT::GetLightDataHandle(),
					(D3D12_CPU_DESCRIPTOR_HANDLE)0,
					(D3D12_CPU_DESCRIPTOR_HANDLE)0,
					(D3D12_CPU_DESCRIPTOR_HANDLE)0,
					ssao ? ssao->GetSRV() : (D3D12_CPU_DESCRIPTOR_HANDLE)0
				};

				D_LIGHT::GetShadowTextureArrayHandle(lightHandles[2], lightHandles[3], lightHandles[4]);

				D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &(CommonTexture[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()]), &destCount, destCount, lightHandles, sourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			}

			// Setup samplers
			context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SamplerHeap.GetHeapPointer());
			context.SetDescriptorTable(kMaterialSamplers, CommonTextureSamplers);

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

			if (m_BatchType == kDefault)
			{
				switch (m_CurrentPass)
				{
				case kZPass:
					context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					context.SetDepthStencilTarget(m_DSV->GetDSV());
					break;
					continue;
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

			while (m_CurrentDraw < lastDraw)
			{
				SortKey key;
				key.value = m_SortKeys[m_CurrentDraw];
				const SortObject& object = m_SortObjects[key.objectIdx];
				RenderItem const& ri = object.renderItem;

				context.SetConstantBuffer(kMeshConstants, ri.MeshCBV);

				if (ri.PsoFlags & RenderItem::ColorOnly)
				{
					D_RENDERER_FRAME_RESOURCE::ColorConstants color = { ri.Color };
					context.SetDynamicConstantBufferView(kMaterialConstants, sizeof(ColorConstants), &color);
				}
				else
				{
					context.SetConstantBuffer(kMaterialConstants, ri.Material.MaterialCBV);
					context.SetDescriptorTable(kMaterialSRVs, ri.Material.MaterialSRV);
				}

				if (ri.mNumJoints > 0)
				{
					context.SetDynamicSRV(kSkinMatrices, sizeof(Joint) * ri.mNumJoints, ri.mJointData);
				}

				context.SetPipelineState(Psos[key.psoIdx]);

				context.SetPrimitiveTopology(ri.PrimitiveType);

				context.SetVertexBuffer(0, ri.Mesh->VertexBufferView());
				context.SetIndexBuffer(ri.Mesh->IndexBufferView());

				// TODO: Render submeshes one by one
				//for (uint32_t i = 0; i < ri.Mesh->mDraw; ++i)
					//context.DrawIndexed(mesh.draw[i].primCount, mesh.draw[i].startIndex, mesh.draw[i].baseVertex);
				context.DrawIndexedInstanced(ri.IndexCount, 1, ri.StartIndexLocation, ri.BaseVertexLocation, 0);

				++m_CurrentDraw;
			}
		}

		context.PIXEndEvent();
	}

#ifdef _D_EDITOR
	DescriptorHandle	AllocateUiTexture(UINT count)
	{
		return ImguiHeap.Alloc(count);
	}

	void RenderGui()
	{
		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Render Gui");

		auto& viewportRt = D_GRAPHICS_DEVICE::GetRTBuffer();
		auto& depthStencil = D_GRAPHICS_DEVICE::GetDepthStencilBuffer();
		context.TransitionResource(depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		context.TransitionResource(viewportRt, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		// Clear RT
		Clear(context, viewportRt, depthStencil, D_GRAPHICS_DEVICE::GetOutputSize());

		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ImguiHeap.GetHeapPointer());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());
		context.Finish();
	}

	void InitializeGUI()
	{
		ImguiHeap.Create(L"Imgui Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);

		auto defualtHandle = ImguiHeap.Alloc(1);

		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGui_ImplWin32_Init(D_GRAPHICS::GetWindow());
		ImGui_ImplDX12_Init(D_GRAPHICS_DEVICE::GetDevice(), D_GRAPHICS_DEVICE::GetBackBufferCount(), D_GRAPHICS::SwapChainGetColorFormat(), ImguiHeap.GetHeapPointer(), defualtHandle, defualtHandle);

		D_GRAPHICS::GetCommandManager()->IdleGPU();
	}
#endif

	uint8_t GetPso(uint16_t psoFlags)
	{
		GraphicsPSO ColorPSO = DefaultPso;
		using namespace D_RENDERER_FRAME_RESOURCE;
		uint16_t Requirements = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0;

		// Checking requirements and supported features
		// Before removing any of unsupported check asserts, make sure the appropriate shader exists
		// and is binded to a pipeline
		D_ASSERT((psoFlags & Requirements) == Requirements);
		D_ASSERT_M(!(psoFlags & RenderItem::HasUV1), "Higher level UV sets are not supported yet");

		// Setting input layout
		if (psoFlags & RenderItem::HasSkin)
			ColorPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned));
		else
			ColorPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture));

#define GET_SHADER(name) Psos[name]

		if (psoFlags & RenderItem::ColorOnly)
		{
			ColorPSO.SetVertexShader(ShaderData("ColorVS"));
			ColorPSO.SetPixelShader(ShaderData("ColorPS"));
		}
		else
		{
			if (psoFlags & RenderItem::HasSkin)
			{
				if (psoFlags & RenderItem::HasTangent)
				{
					if (psoFlags & RenderItem::HasUV1)
					{
						// TODO: Change to a shader supporting UV1
						ColorPSO.SetVertexShader(ShaderData("SkinnedVS"));
						ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
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
					if (psoFlags & RenderItem::HasUV1)
					{
						ColorPSO.SetVertexShader(ShaderData("SkinnedVS"));
						ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
					}
					else
					{
						ColorPSO.SetVertexShader(ShaderData("SkinnedVS"));
						ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
					}
				}
			}
			else
			{
				if (psoFlags & RenderItem::HasTangent)
				{
					if (psoFlags & RenderItem::HasUV1)
					{
						// TODO: Change to a shader supporting UV1
						ColorPSO.SetVertexShader(ShaderData("DefaultVS"));
						ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
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
					if (psoFlags & RenderItem::HasUV1)
					{
						D_ASSERT_M(true, "Higher level UV sets are not supported yet");
						ColorPSO.SetVertexShader(ShaderData("DefaultVS"));
						ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
					}
					else
					{
						ColorPSO.SetVertexShader(ShaderData("DefaultVS"));
						ColorPSO.SetPixelShader(ShaderData("DefaultPS"));
					}
				}
			}
		}

		if (psoFlags & RenderItem::AlphaBlend)
		{
			ColorPSO.SetBlendState(BlendTraditional);
			ColorPSO.SetDepthStencilState(DepthStateReadOnly);
		}
		if (psoFlags & RenderItem::TwoSided)
		{
			if (psoFlags & RenderItem::Wireframe)
				ColorPSO.SetRasterizerState(RasterizerTwoSidedWireframe);
			else
				ColorPSO.SetRasterizerState(RasterizerTwoSided);
		}
		else
		{
			if (psoFlags & RenderItem::Wireframe)
				ColorPSO.SetRasterizerState(RasterizerDefaultWireframe);
			else
				ColorPSO.SetRasterizerState(RasterizerDefault);
		}

		if (psoFlags & RenderItem::LineOnly)
			ColorPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		else
			ColorPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		ColorPSO.Finalize();

		// Look for an existing PSO
		for (uint32_t i = 0; i < Psos.size(); ++i)
		{
			if (ColorPSO.GetPipelineStateObject() == Psos[i].GetPipelineStateObject())
			{
				return (uint8_t)i;
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

		D_ASSERT_M(Psos.size() <= 256, "Ran out of room for unique PSOs");

		return (uint8_t)Psos.size() - 2;
	}


}