#include "Renderer.hpp"
#include "pch.hpp"

#include "Renderer.hpp"
#include "Geometry/Mesh.hpp"
#include "FrameResource.hpp"
#include "GraphicsCore.hpp"
#include "GraphicsUtils/D3DUtils.hpp"
#include "GraphicsUtils/RootSignature.hpp"
#include "GraphicsUtils/Memory/DescriptorHeap.hpp"
#include "GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "GraphicsUtils/VertexTypes.hpp"
#include "Camera/CameraManager.hpp"
#include "GraphicsUtils/Profiling/Profiling.hpp"
#include "Resources/TextureResource.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Math/VectorMath.hpp>
#include <Utils/Assert.hpp>

#include <imgui.h>
#include <implot/implot.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include <filesystem>


using namespace Microsoft::WRL;

using namespace Darius::Math;
using namespace Darius::Renderer::DeviceResource;
using namespace Darius::Renderer::GraphicsUtils;
using namespace D_RENDERER_GEOMETRY;
using namespace D_GRAPHICS_MEMORY;
using namespace D_GRAPHICS_UTILS;

#define VertexData(il) il::InputLayout.NumElements, il::InputLayout.pInputElementDescs
#define ShaderData(name) Shaders[name]->GetBufferPointer(), Shaders[name]->GetBufferSize()

namespace Darius::Renderer
{

	ID3D12Device* _device = nullptr;

#ifdef _D_EDITOR
	DescriptorHeap										ImguiHeap;
	UINT												MaxImguiElements = 2;
#endif

	// Input layout and root signature
	std::array<D_GRAPHICS_UTILS::RootSignature, (size_t)RootSignatureTypes::_numRootSig> RootSigns;
	DVector<D_GRAPHICS_UTILS::GraphicsPSO> Psos(18);

	// Device resource
	std::unique_ptr<DeviceResource::DeviceResources>	Resources;

	DescriptorHeap										TextureHeap;
	DescriptorHeap										SamplerHeap;

	DescriptorHandle									CommonTexture;
	DescriptorHandle									CommonTextureSamplers;

	D_CORE::Ref<TextureResource>						RadianceCubeMap;
	D_CORE::Ref<TextureResource>						IrradianceCubeMap;
	D_CORE::Ref<TextureResource>						DefaultBlackCubeMap;
	float												SpecularIBLRange;
	float												SpecularIBLBias = FLT_MAX;

	//////////////////////////////////////////////////////
	// Options
	bool												SeparateZPass = false;

	//////////////////////////////////////////////////////
	// Functions
#ifdef _D_EDITOR
	void InitializeGUI();
#endif
	void BuildPSO();
	void BuildRootSignature();

	enum PipelineStateTypes
	{
		OpaquePso,
		TransparentPso,
		WireframePso,
		SkinnedOpaquePso,
		SkinnedTransparentPso,
		SkinnedWireframePso,
		ColorPso,
		ColorWireframePso,
		ColorWireframeTwoSidedPso,
		SkyboxPso,
		DepthOnlyPso,
		CutoutDepthPso,
		SkinDepthOnlyPso,
		SkinCutoutDepthPso,
		ShadowDepthOnlyPso,
		ShadowCutoutDepthPso,
		ShadowSkinDepthOnlyPso,
		ShadowSkinCutoutDepthPso,

	};


	void Clear(D_GRAPHICS::GraphicsContext& context, D_GRAPHICS_BUFFERS::ColorBuffer& rt, D_GRAPHICS_BUFFERS::DepthBuffer& depthStencil, RECT bounds, std::wstring const& processName = L"Clear");

	void Initialize()
	{
		D_ASSERT(_device == nullptr);
		D_ASSERT(Resources);

		_device = Resources->GetD3DDevice();

		BuildRootSignature();
		BuildPSO();

		TextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
		SamplerHeap.Create(L"Scene Sampler Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);

		CommonTexture = TextureHeap.Alloc(8);
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

			DescriptorHandle dest = CommonTexture + 2 * TextureHeap.GetDescriptorSize();

			_device->CopyDescriptors(1, &dest, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}

	void Shutdown()
	{
		D_ASSERT(_device != nullptr);

		ImguiHeap.Destroy();
		TextureHeap.Destroy();
		SamplerHeap.Destroy();
		D_GRAPHICS_UTILS::RootSignature::DestroyAll();
		D_GRAPHICS_UTILS::GraphicsPSO::DestroyAll();
	}

	D_GRAPHICS_UTILS::RootSignature& GetRootSignature(RootSignatureTypes type)
	{
		return RootSigns[(size_t)type];
	}

	void Present()
	{
		// Show the new frame.
		Resources->Present();
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
		auto viewport = CD3DX12_VIEWPORT((float)bounds.left, (float)bounds.top, (long)width, (long)height, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissorRect = CD3DX12_RECT(bounds.left, bounds.top, (long)width, (long)height);


		context.ClearColor(rt, &scissorRect);
		context.ClearDepth(depthStencil);
		context.SetRenderTarget(rtvDescriptor, dsvDescriptor);
		context.SetViewportAndScissor(viewport, scissorRect);

		PIXEndEvent(context.GetCommandList());
	}

	void BuildPSO()
	{

		DXGI_FORMAT ColorFormat = Resources->GetBackBufferFormat();
		DXGI_FORMAT DepthFormat = Resources->GetDepthBufferFormat();
		DXGI_FORMAT ShadowFormat = Resources->GetShadowBufferFormat();

		// For Opaque objects
		Psos[(size_t)PipelineStateTypes::OpaquePso] = GraphicsPSO(L"Opaque PSO");
		auto& pso = Psos[(size_t)PipelineStateTypes::OpaquePso];

		auto il = D_GRAPHICS_VERTEX::VertexPositionNormalTangentTexture::InputLayout;
		pso.SetInputLayout(il.NumElements, il.pInputElementDescs);

		pso.SetVertexShader(reinterpret_cast<BYTE*>(Shaders["DefaultVS"]->GetBufferPointer()),
			Shaders["DefaultVS"]->GetBufferSize());
		pso.SetPixelShader(reinterpret_cast<BYTE*>(Shaders["DefaultPS"]->GetBufferPointer()),
			Shaders["DefaultPS"]->GetBufferSize());
		pso.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		pso.SetRasterizerState(D_GRAPHICS::RasterizerDefault);
		pso.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		pso.SetDepthStencilState(DepthStateReadWrite);
		pso.SetSampleMask(UINT_MAX);
		pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		pso.SetRenderTargetFormat(ColorFormat, DepthFormat);
		pso.Finalize();

		// Transparent
		Psos[(size_t)PipelineStateTypes::TransparentPso] = GraphicsPSO(L"Transparent PSO");
		auto& transPso = Psos[(size_t)PipelineStateTypes::TransparentPso];
		transPso = pso;
		transPso.SetBlendState(D_GRAPHICS::BlendTraditional);
		transPso.Finalize();

		// For opaque wireframe objecs
		Psos[(size_t)PipelineStateTypes::WireframePso] = pso;
		auto& wirePso = Psos[(size_t)PipelineStateTypes::WireframePso];
		auto wireRasterState = D_GRAPHICS::RasterizerDefaultWireframe;
		wireRasterState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		wirePso.SetRasterizerState(wireRasterState);
		wirePso.Finalize(L"Wireframe");

		// For Skinned Transparent
		{
			auto skinnedOpaquePso = GraphicsPSO(L"Skinned Opaque PSO");
			skinnedOpaquePso = pso;
			skinnedOpaquePso.SetVertexShader(Shaders["SkinnedVS"]->GetBufferPointer(), Shaders["SkinnedVS"]->GetBufferSize());
			skinnedOpaquePso.SetInputLayout(D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned::InputLayout.NumElements, D_GRAPHICS_VERTEX::VertexPositionNormalTangentTextureSkinned::InputLayout.pInputElementDescs);
			skinnedOpaquePso.Finalize();
			Psos[(size_t)PipelineStateTypes::SkinnedOpaquePso] = skinnedOpaquePso;
		}

		// For colored only objects
		Psos[(size_t)PipelineStateTypes::ColorPso] = pso;
		auto& colorPso = Psos[(size_t)PipelineStateTypes::ColorPso];
		il = D_GRAPHICS_VERTEX::VertexPosition::InputLayout;
		colorPso.SetInputLayout(il.NumElements, il.pInputElementDescs);
		colorPso.SetVertexShader(reinterpret_cast<BYTE*>(Shaders["ColorVS"]->GetBufferPointer()), Shaders["ColorVS"]->GetBufferSize());
		colorPso.SetPixelShader(reinterpret_cast<BYTE*>(Shaders["ColorPS"]->GetBufferPointer()), Shaders["ColorPS"]->GetBufferSize());
		colorPso.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		colorPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		colorPso.Finalize(L"Color");

		{
			// For colored wireframe objecs
			Psos[(size_t)PipelineStateTypes::ColorWireframePso] = Psos[(size_t)PipelineStateTypes::ColorPso];
			auto& wireColorPso = Psos[(size_t)PipelineStateTypes::ColorWireframePso];
			wireColorPso.SetRasterizerState(D_GRAPHICS::RasterizerDefaultWireframe);
			wireColorPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
			wireColorPso.Finalize(L"Color Wireframe");
		}

		{
			// For colored wireframe objecs
			Psos[(size_t)PipelineStateTypes::ColorWireframeTwoSidedPso] = Psos[(size_t)PipelineStateTypes::ColorWireframePso];
			auto& wireColorPso = Psos[(size_t)PipelineStateTypes::ColorWireframeTwoSidedPso];
			wireColorPso.SetRasterizerState(D_GRAPHICS::RasterizerTwoSidedMsaaWireframe);
			wireColorPso.Finalize(L"Color Wireframe Two Sided");
		}

		// Skybox
		{
			Psos[(size_t)PipelineStateTypes::SkyboxPso] = Psos[(size_t)PipelineStateTypes::OpaquePso];
			auto& skyboxPso = Psos[(size_t)PipelineStateTypes::SkyboxPso];
			skyboxPso.SetDepthStencilState(DepthStateTestEqual);
			skyboxPso.SetInputLayout(0, nullptr);
			skyboxPso.SetVertexShader(reinterpret_cast<BYTE*>(Shaders["SkyboxVS"]->GetBufferPointer()), Shaders["SkyboxVS"]->GetBufferSize());
			skyboxPso.SetPixelShader(reinterpret_cast<BYTE*>(Shaders["SkyboxPS"]->GetBufferPointer()), Shaders["SkyboxPS"]->GetBufferSize());
			skyboxPso.Finalize(L"Skybox");
		}

		// Depth Only PSOs

		GraphicsPSO DepthOnlyPSO(L"Renderer: Depth Only PSO");
		DepthOnlyPSO.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		DepthOnlyPSO.SetRasterizerState(RasterizerDefault);
		DepthOnlyPSO.SetBlendState(BlendDisable);
		DepthOnlyPSO.SetDepthStencilState(DepthStateReadWrite);
		DepthOnlyPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPosition));
		DepthOnlyPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		DepthOnlyPSO.SetRenderTargetFormats(0, nullptr, DepthFormat);
		DepthOnlyPSO.SetVertexShader(ShaderData("DepthOnlyVS"));
		DepthOnlyPSO.Finalize();
		Psos[(size_t)PipelineStateTypes::DepthOnlyPso] = DepthOnlyPSO;

		GraphicsPSO CutoutDepthPSO(L"Renderer: Cutout Depth PSO");
		CutoutDepthPSO = DepthOnlyPSO;
		CutoutDepthPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionTexture));
		CutoutDepthPSO.SetRasterizerState(RasterizerTwoSided);
		CutoutDepthPSO.SetVertexShader(ShaderData("CutoutDepthVS"));
		CutoutDepthPSO.SetPixelShader(ShaderData("CutoutDepthPS"));
		CutoutDepthPSO.Finalize();
		Psos[(size_t)PipelineStateTypes::CutoutDepthPso] = CutoutDepthPSO;

		GraphicsPSO SkinDepthOnlyPSO = DepthOnlyPSO;
		SkinDepthOnlyPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionSkinned));
		SkinDepthOnlyPSO.SetVertexShader(ShaderData("DepthOnlySkinVS"));
		SkinDepthOnlyPSO.Finalize();
		Psos[(size_t)PipelineStateTypes::SkinDepthOnlyPso] = SkinDepthOnlyPSO;

		GraphicsPSO SkinCutoutDepthPSO = CutoutDepthPSO;
		SkinCutoutDepthPSO.SetInputLayout(VertexData(D_GRAPHICS_VERTEX::VertexPositionTextureSkinned));
		SkinCutoutDepthPSO.SetVertexShader(ShaderData("CutoutDepthSkinVS"));
		SkinCutoutDepthPSO.Finalize();
		Psos[(size_t)PipelineStateTypes::SkinCutoutDepthPso] = SkinCutoutDepthPSO;

		// Shadow PSOs

		DepthOnlyPSO.SetRasterizerState(RasterizerShadow);
		DepthOnlyPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		DepthOnlyPSO.Finalize();
		Psos[(size_t)PipelineStateTypes::ShadowDepthOnlyPso] = DepthOnlyPSO;

		CutoutDepthPSO.SetRasterizerState(RasterizerShadowTwoSided);
		CutoutDepthPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		CutoutDepthPSO.Finalize();
		Psos[(size_t)PipelineStateTypes::ShadowCutoutDepthPso] = CutoutDepthPSO;

		SkinDepthOnlyPSO.SetRasterizerState(RasterizerShadow);
		SkinDepthOnlyPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		SkinDepthOnlyPSO.Finalize();
		Psos[(size_t)PipelineStateTypes::ShadowSkinDepthOnlyPso] = SkinDepthOnlyPSO;

		SkinCutoutDepthPSO.SetRasterizerState(RasterizerShadowTwoSided);
		SkinCutoutDepthPSO.SetRenderTargetFormats(0, nullptr, ShadowFormat);
		SkinCutoutDepthPSO.Finalize();
		Psos[(size_t)PipelineStateTypes::ShadowSkinCutoutDepthPso] = SkinCutoutDepthPSO;
	}

	void BuildRootSignature()
	{
		// Default root signature
		auto& def = RootSigns[(size_t)RootSignatureTypes::DefaultRootSig];
		def.Reset(kNumRootBindings, 3);

		// Create samplers
		SamplerDesc defaultSamplerDesc;
		defaultSamplerDesc.MaxAnisotropy = 8;
		SamplerDesc cubeMapSamplerDesc = defaultSamplerDesc;
		def.InitStaticSampler(10, defaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(11, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(12, cubeMapSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);

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

		DescriptorHandle dest = CommonTexture + 2 * TextureHeap.GetDescriptorSize();

		_device->CopyDescriptors(1, &dest, &DestCount, DestCount, SourceTextures, SourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	void DrawSkybox(D_GRAPHICS::GraphicsContext& context, const D_MATH_CAMERA::Camera& camera, D_GRAPHICS_BUFFERS::ColorBuffer& sceneColor, D_GRAPHICS_BUFFERS::DepthBuffer& sceneDepth, const D3D12_VIEWPORT& viewport, const D3D12_RECT& scissor)
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
		context.SetPipelineState(Psos[(size_t)PipelineStateTypes::SkyboxPso]);

		// Transition of render targets
		context.TransitionResource(sceneDepth, D3D12_RESOURCE_STATE_DEPTH_READ);
		context.TransitionResource(sceneColor, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		context.SetRenderTarget(sceneColor.GetRTV(), sceneDepth.GetDSV_DepthReadOnly());
		context.SetViewportAndScissor(viewport, scissor);

		// Bind pipeline resources
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TextureHeap.GetHeapPointer());
		context.SetDynamicConstantBufferView(kMeshConstants, sizeof(SkyboxVSCB), &skyVSCB);
		context.SetDynamicConstantBufferView(kMaterialConstants, sizeof(SkyboxPSCB), &skyPSVB);
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
		bool skinned = (renderItem.PsoFlags & RenderItem::HasSkin) == RenderItem::HasSkin;
		uint64_t depthPSO = (skinned ? 2 : 0) + (alphaTest ? 1 : 0);

		union float_or_int { float f; uint32_t u; } dist;
		dist.f = Max(distance, 0.0f);

		if (m_BatchType == kShadows)
		{
			D_ASSERT(false);
			if (alphaBlend)
				return;

			key.passID = kZPass;
			key.psoIdx = depthPSO + 4;
			key.key = dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kZPass]++;
		}
		else if (renderItem.PsoFlags & RenderItem::AlphaBlend)
		{
			key.passID = kTransparent;
			key.psoIdx = renderItem.PsoType;
			key.key = ~dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kTransparent]++;
		}
		else if (SeparateZPass || alphaTest)
		{
			D_ASSERT(false);
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
		GlobalConstants& globals)
	{
		D_ASSERT(m_DSV != nullptr);

		context.PIXBeginEvent(L"Render Meshes");

		context.SetRootSignature(RootSigns[DefaultRootSig]);

		globals.IBLBias = SpecularIBLBias;
		globals.IBLRange = SpecularIBLRange;
		context.SetDynamicConstantBufferView(kCommonCBV, sizeof(GlobalConstants), &globals);

		// Setting up common texture (light for now)
		UINT destCount = 2;
		UINT sourceCounts[] = { 1, 1 };
		D3D12_CPU_DESCRIPTOR_HANDLE lightHandles[] =
		{
			D_LIGHT::GetLightMaskHandle(),
			D_LIGHT::GetLightDataHandle()
		};
		_device->CopyDescriptors(1, &CommonTexture, &destCount, destCount, lightHandles, sourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TextureHeap.GetHeapPointer());
		context.SetDescriptorTable(kCommonSRVs, CommonTexture);

		// Setup samplers
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SamplerHeap.GetHeapPointer());
		context.SetDescriptorTable(kMaterialSamplers, CommonTextureSamplers);

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
					/*context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					context.SetDepthStencilTarget(m_DSV->GetDSV());
					break;*/
					continue;
				case kOpaque:
					context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					context.TransitionResource(*m_RTV[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
					context.SetRenderTarget(m_RTV[0]->GetRTV(), m_DSV->GetDSV());
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

				context.SetPrimitiveTopology(ri.PrimitiveType);

				context.SetConstantBuffer(kMeshConstants, ri.MeshCBV);

				if (ri.PsoFlags & RenderItem::ColorOnly)
				{
					D_RENDERER_FRAME_RESOUCE::ColorConstants color = { ri.Color };
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

				context.SetVertexBuffer(0, ri.Mesh->VertexBufferView());
				context.SetIndexBuffer(ri.Mesh->IndexBufferView());

				// TODO: Render submeshes one by one
				//for (uint32_t i = 0; i < ri.Mesh->mDraw; ++i)
					//context.DrawIndexed(mesh.draw[i].primCount, mesh.draw[i].startIndex, mesh.draw[i].baseVertex);
				context.DrawIndexedInstanced(ri.IndexCount, 1, ri.StartIndexLocation, ri.BaseVertexLocation, 0);

				++m_CurrentDraw;
			}
		}

		if (m_BatchType == kShadows)
		{
			context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}

		context.PIXEndEvent();
	}

#ifdef _D_EDITOR
	DescriptorHandle	GetUiTextureHandle(UINT index)
	{
		D_ASSERT_M(index != 0, "Access to 0 index of render resouces are not allowed");
		D_ASSERT_M(index < MaxImguiElements&& index >= 1, "Index out of bound");
		return ImguiHeap[index];
	}

	void RenderGui()
	{
		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Render Gui");

		auto& viewportRt = Resources->GetRTBuffer();
		auto& depthStencil = Resources->GetDepthStencilBuffer();
		context.TransitionResource(depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		context.TransitionResource(viewportRt, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		// Clear RT
		Clear(context, viewportRt, depthStencil, Resources->GetOutputSize());

		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ImguiHeap.GetHeapPointer());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());
		context.Finish();
	}

	void InitializeGUI()
	{
		ImguiHeap.Create(L"Imgui Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, MaxImguiElements);

		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGui_ImplWin32_Init(Resources->GetWindow());
		ImGui_ImplDX12_Init(_device, Resources->GetBackBufferCount(), DXGI_FORMAT_B8G8R8A8_UNORM, ImguiHeap.GetHeapPointer(), ImguiHeap.GetHeapPointer()->GetCPUDescriptorHandleForHeapStart(), ImguiHeap.GetHeapPointer()->GetGPUDescriptorHandleForHeapStart());
	}
#endif

	namespace Device
	{
		void Initialize(HWND window, int width, int height)
		{
			// TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
			//   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
			//   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
			Resources = std::make_unique<DeviceResource::DeviceResources>();

			Resources->SetWindow(window, width, height);
			Resources->CreateDeviceResources();
			D_GRAPHICS::Initialize();
			Resources->CreateWindowSizeDependentResources();
			D_CAMERA_MANAGER::Initialize();
			D_CAMERA_MANAGER::SetViewportDimansion((float)width, (float)height);
		}

		void RegisterDeviceNotify(IDeviceNotify* notify)
		{
			Resources->RegisterDeviceNotify(notify);
		}

		void Shutdown()
		{
			Resources.reset();
			D_GRAPHICS::Shutdown();
			D_CAMERA_MANAGER::Shutdown();
		}

		void OnWindowMoved()
		{
			auto const r = Resources->GetOutputSize();
			Resources->WindowSizeChanged(r.right, r.bottom);
		}

		void OnDisplayChanged()
		{
			Resources->UpdateColorSpace();
		}

		bool OnWindowsSizeChanged(int width, int height)
		{
			return Resources->WindowSizeChanged(width, height);
		}

		void ShaderCompatibilityCheck(D3D_SHADER_MODEL shader)
		{
			auto device = Resources->GetD3DDevice();

			// Check Shader Model 6 support
			D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { shader };
			if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
				|| (shaderModel.HighestShaderModel < shader))
			{
#ifdef _DEBUG
				OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
				throw std::runtime_error("Shader Model 6.0 is not supported!");
			}
		}

		FrameResource* GetCurrentFrameResource()
		{
			return Resources->GetFrameResource();
		}

		UINT GetCurrentResourceIndex()
		{
			return Resources->GetCurrentFrameResourceIndex();
		}

		ID3D12Device* GetDevice()
		{
			return Resources->GetD3DDevice();
		}

		FrameResource* GetFrameResourceWithIndex(int i)
		{
			return Resources->GetFrameResourceByIndex(i);
		}

		DXGI_FORMAT GetBackBufferFormat()
		{
			return Resources->GetBackBufferFormat();
		}

		DXGI_FORMAT GetDepthBufferFormat()
		{
			return Resources->GetDepthBufferFormat();
		}

		void WaitForGpu()
		{
			auto cmdManager = D_GRAPHICS::GetCommandManager();
			cmdManager->IdleGPU();
		}
	}


	uint8_t GetPso(uint16_t psoFlags)
	{
		GraphicsPSO ColorPSO = Psos[OpaquePso];
		using namespace D_RENDERER_FRAME_RESOUCE;
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
				ColorPSO.SetRasterizerState(RasterizerTwoSidedMsaaWireframe);
			else
				ColorPSO.SetRasterizerState(RasterizerTwoSidedMsaa);
		}
		else
		{
			if (psoFlags & RenderItem::Wireframe)
				ColorPSO.SetRasterizerState(RasterizerDefaultMsaaWireframe);
			else
				ColorPSO.SetRasterizerState(RasterizerDefaultMsaa);
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