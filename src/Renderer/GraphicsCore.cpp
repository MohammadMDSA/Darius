#include "pch.hpp"
#include "GraphicsCore.hpp"

#include "AntiAliasing/TemporalEffect.hpp"
#include "AmbientOcclusion/ScreenSpaceAmbientOcclusion.hpp"
#include "CommandContext.hpp"
#include "Components/LightComponent.hpp"
#include "Components/BillboardRendererComponent.hpp"
#include "Components/MeshRendererComponent.hpp"
#include "Components/SkeletalMeshRendererComponent.hpp"
#include "Components/TerrainRendererComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Geometry/GeometryGenerator.hpp"
#include "GraphicsDeviceManager.hpp"
#include "GraphicsUtils/Buffers/Texture.hpp"
#include "GraphicsUtils/D3DUtils.hpp"
#include "GraphicsUtils/Memory/DescriptorHeap.hpp"
#include "GraphicsUtils/Profiling/GpuTimeManager.hpp"
#include "Light/LightManager.hpp"
#include "PostProcessing/MotionBlur.hpp"
#include "PostProcessing/PostProcessing.hpp"
#include "Renderer.hpp"
#include "Resources/BatchResource.hpp"
#include "Resources/MaterialResource.hpp"
#include "Resources/StaticMeshResource.hpp"
#include "Resources/SkeletalMeshResource.hpp"
#include "Resources/TerrainResource.hpp"
#include "Resources/TextureResource.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#include <dxcapi.h>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

using namespace D_CONTAINERS;
using namespace D_CORE;
using namespace D_FILE;
using namespace D_GRAPHICS_MEMORY;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_GRAPHICS_UTILS;
using namespace D_MATH;
using namespace D_RENDERER_GEOMETRY;
using namespace D_RESOURCE;
using namespace DirectX;
using namespace Microsoft::WRL;

namespace Darius::Graphics
{
	bool _initialized = false;
	CommandListManager					CommandManager;
	ContextManager						CtxManager;
	DescriptorAllocator					DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV
	};

	std::vector<ComPtr<ID3DBlob>>						Shaders;
	std::unordered_map<std::string, UINT32>				ShaderNameMap;

	DUnorderedMap<DefaultResource, ResourceHandle>		DefaultResourceMap;

	uint32_t											FrameCount = 0;

	// Formats
	DXGI_FORMAT											ColorFormat = DXGI_FORMAT_R11G11B10_FLOAT;
	DXGI_FORMAT											DepthFormat = DXGI_FORMAT_D32_FLOAT;
	DXGI_FORMAT											ShadowFormat = DXGI_FORMAT_D16_UNORM;

	// Device resource
	std::unique_ptr<Device::DeviceResources>			Resources;

	//////////////////////////////////
	////////// Common States /////////
	//////////////////////////////////

	SamplerDesc								SamplerLinearWrapDesc;
	SamplerDesc								SamplerAnisoWrapDesc;
	SamplerDesc								SamplerShadowDesc;
	SamplerDesc								SamplerLinearClampDesc;
	SamplerDesc								SamplerVolumeWrapDesc;
	SamplerDesc								SamplerPointClampDesc;
	SamplerDesc								SamplerPointBorderDesc;
	SamplerDesc								SamplerLinearBorderDesc;

	D3D12_CPU_DESCRIPTOR_HANDLE				SamplerLinearWrap;
	D3D12_CPU_DESCRIPTOR_HANDLE				SamplerAnisoWrap;
	D3D12_CPU_DESCRIPTOR_HANDLE				SamplerShadow;
	D3D12_CPU_DESCRIPTOR_HANDLE				SamplerLinearClamp;
	D3D12_CPU_DESCRIPTOR_HANDLE				SamplerVolumeWrap;
	D3D12_CPU_DESCRIPTOR_HANDLE				SamplerPointClamp;
	D3D12_CPU_DESCRIPTOR_HANDLE				SamplerPointBorder;
	D3D12_CPU_DESCRIPTOR_HANDLE				SamplerLinearBorder;

	D3D12_RASTERIZER_DESC					RasterizerDefault;	// Counter-clockwise
	D3D12_RASTERIZER_DESC					RasterizerDefaultWireframe;
	D3D12_RASTERIZER_DESC					RasterizerDefaultMsaa;
	D3D12_RASTERIZER_DESC					RasterizerDefaultMsaaWireframe;
	D3D12_RASTERIZER_DESC					RasterizerDefaultCw;	// Clockwise winding
	D3D12_RASTERIZER_DESC					RasterizerDefaultCwWireframe;
	D3D12_RASTERIZER_DESC					RasterizerDefaultCwMsaa;
	D3D12_RASTERIZER_DESC					RasterizerDefaultCwMsaaWireframe;
	D3D12_RASTERIZER_DESC					RasterizerTwoSided;
	D3D12_RASTERIZER_DESC					RasterizerTwoSidedWireframe;
	D3D12_RASTERIZER_DESC					RasterizerTwoSidedMsaa;
	D3D12_RASTERIZER_DESC					RasterizerTwoSidedMsaaWireframe;
	D3D12_RASTERIZER_DESC					RasterizerShadow;
	D3D12_RASTERIZER_DESC					RasterizerShadowCW;
	D3D12_RASTERIZER_DESC					RasterizerShadowTwoSided;
	D3D12_RASTERIZER_DESC					RasterizerShadowWireframe;
	D3D12_RASTERIZER_DESC					RasterizerShadowCWWireframe;
	D3D12_RASTERIZER_DESC					RasterizerShadowTwoSidedWireframe;

	D3D12_BLEND_DESC						BlendNoColorWrite;
	D3D12_BLEND_DESC						BlendDisable;
	D3D12_BLEND_DESC						BlendPreMultiplied;
	D3D12_BLEND_DESC						BlendTraditional;
	D3D12_BLEND_DESC						BlendAdditive;
	D3D12_BLEND_DESC						BlendTraditionalAdditive;

	D3D12_DEPTH_STENCIL_DESC				DepthStateDisabled;
	D3D12_DEPTH_STENCIL_DESC				DepthStateReadWrite;
	D3D12_DEPTH_STENCIL_DESC				DepthStateReadOnly;
	D3D12_DEPTH_STENCIL_DESC				DepthStateReadOnlyReversed;
	D3D12_DEPTH_STENCIL_DESC				DepthStateTestEqual;

	CommandSignature						DispatchIndirectCommandSignature(1);
	CommandSignature						DrawIndirectCommandSignature(1);

	D_GRAPHICS_UTILS::RootSignature			CommonRS;

	ComputePSO								GenerateMipsLinearPSO[4] =
	{
		{L"Generate Mips Linear CS"},
		{L"Generate Mips Linear Odd X CS"},
		{L"Generate Mips Linear Odd Y CS"},
		{L"Generate Mips Linear Odd CS"},
	};

	ComputePSO GenerateMipsGammaPSO[4] =
	{
		{ L"Generate Mips Gamma CS" },
		{ L"Generate Mips Gamma Odd X CS" },
		{ L"Generate Mips Gamma Odd Y CS" },
		{ L"Generate Mips Gamma Odd CS" },
	};

	/*GraphicsPSO DownsampleDepthPSO(L"DownsampleDepth PSO")*/;


	namespace Device
	{
		void Initialize(HWND window, int width, int height, D_SERIALIZATION::Json const& settings)
		{
			// Initialize Device

			// TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
			//   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
			//   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
			Resources = std::make_unique<DeviceResources>();

			Resources->SetWindow(window, width, height);
			Resources->CreateDeviceResources();

		}

		void RegisterDeviceNotify(void* notify)
		{
			Resources->RegisterDeviceNotify(reinterpret_cast<IDeviceNotify*>(notify));
		}

		void Shutdown()
		{
			Resources.reset();
			D_GRAPHICS::Shutdown();
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

		UINT GetCurrentFrameResourceIndex()
		{
			return Resources->GetCurrentFrameResourceIndex();
		}


		RECT GetOutputSize()
		{
			return Resources->GetOutputSize();
		}

		ID3D12Device* GetDevice()
		{
			return Resources->GetD3DDevice();
		}

		DXGI_FORMAT GetBackBufferFormat()
		{
			return Resources->GetBackBufferFormat();
		}

		DXGI_FORMAT GetDepthBufferFormat()
		{
			return Resources->GetDepthBufferFormat();
		}

		D_GRAPHICS_BUFFERS::ColorBuffer& GetRTBuffer()
		{
			return Resources->GetRTBuffer();
		}

		D_GRAPHICS_BUFFERS::DepthBuffer& GetDepthStencilBuffer()
		{
			return Resources->GetDepthStencilBuffer();
		}

		UINT GetBackBufferCount()
		{
			return Resources->GetBackBufferCount();
		}

		bool SupportsTypedUAVLoadSupport_R11G11B10_FLOAT()
		{
			return Resources->SupportsTypedUAVLoadSupport_R11G11B10_FLOAT();
		}

		bool SupportsTypedUAVLoadSupport_R16G16B16A16_FLOAT()
		{
			return Resources->SupportsTypedUAVLoadSupport_R16G16B16A16_FLOAT();
		}

		bool SupportsRaytracing()
		{
			return Resources->SupportsRaytracing();
		}

	}


	void InitializeCommonStates();
	void DestroyCommonStates();
	void BuildShaders();
	void LoadDefaultResources();

	void Initialize(HWND window, int width, int height, D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;
		
		Device::Initialize(window, width, height, settings);
		D_ASSERT(Resources);
		D_ASSERT(Resources->GetD3DDevice());

		CommandManager.Create(Device::GetDevice());

		Resources->CreateWindowSizeDependentResources();

		DispatchIndirectCommandSignature[0].Dispatch();
		DispatchIndirectCommandSignature.Finalize();

		DrawIndirectCommandSignature[0].Draw();
		DrawIndirectCommandSignature.Finalize();

		D_PROFILING_GPU::Initialize(4096);

		BuildShaders();
		InitializeCommonStates();

		// Initializing Resources
		TextureResource::Register();
		StaticMeshResource::Register();
		SkeletalMeshResource::Register();
		MaterialResource::Register();
		BatchResource::Register();
		TerrainResource::Register();

		LoadDefaultResources();

		// Setting Main Modules
		D_GRAPHICS_AA_TEMPORAL::Initialize(settings);
		D_GRAPHICS_PP::Initialize(settings);
		D_GRAPHICS_PP_MOTION::Initialize(settings);
		D_GRAPHICS_AO_SS::Initialize(settings);
		D_LIGHT::Initialize();
		D_RENDERER::Initialize(settings);

		// Registering components
		D_GRAPHICS::BillboardRendererComponent::StaticConstructor();
		D_GRAPHICS::CameraComponent::StaticConstructor();
		D_GRAPHICS::LightComponent::StaticConstructor();
		D_GRAPHICS::MeshRendererComponent::StaticConstructor();
		D_GRAPHICS::SkeletalMeshRendererComponent::StaticConstructor();
		D_GRAPHICS::TerrainRendererComponent::StaticConstructor();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		D_RENDERER::Shutdown();

		D_LIGHT::Shutdown();

		D_GRAPHICS_AO_SS::Shutdown();
		D_GRAPHICS_PP_MOTION::Shutdown();
		D_GRAPHICS_PP::Shutdown();
		D_GRAPHICS_AA_TEMPORAL::Shutdown();

		D_PROFILING_GPU::Shutdown();

		CommandManager.IdleGPU();
		CommandContext::DestroyAllContexts();
		CommandManager.Shutdown();

		DescriptorAllocator::DestroyAll();
		PSO::DestroyAll();
		RootSignature::DestroyAll();
		DescriptorAllocator::DestroyAll();

		DestroyCommonStates();

		for (auto& kv : Shaders)
			kv.Reset();

		Device::Shutdown();
	}

	void Present()
	{
		// Show the new frame.
		Resources->Present();

		auto frame = ++FrameCount;
		D_GRAPHICS_AA_TEMPORAL::Update(frame);
	}

	HWND GetWindow()
	{
		return Resources->GetWindow();
	}

	DXGI_FORMAT GetColorFormat()
	{
		return ColorFormat;
	}

	DXGI_FORMAT GetDepthFormat()
	{
		return DepthFormat;
	}

	DXGI_FORMAT GetShadowFormat()
	{
		return ShadowFormat;
	}

	DXGI_FORMAT SwapChainGetColorFormat()
	{
		return Resources->GetBackBufferFormat();
	}

	DXGI_FORMAT SwapChainGetDepthFormat()
	{
		return Resources->GetDepthBufferFormat();
	}

	D_RESOURCE::ResourceHandle GetDefaultGraphicsResource(DefaultResource type)
	{
		return DefaultResourceMap[type];
	}

	uint32_t GetFrameCount()
	{
		return FrameCount;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count)
	{
		return DescriptorAllocators[type].Allocate(count);
	}

	CommandListManager* GetCommandManager()
	{
		D_ASSERT(_initialized);
		return &CommandManager;
	}

	ContextManager* GetContextManager()
	{
		D_ASSERT(_initialized);
		return &CtxManager;
	}

	void InitializeCommonStates()
	{
		SamplerLinearWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		SamplerLinearWrap = SamplerLinearWrapDesc.CreateDescriptor();

		SamplerAnisoWrapDesc.MaxAnisotropy = 4;
		SamplerAnisoWrap = SamplerAnisoWrapDesc.CreateDescriptor();

		SamplerShadowDesc.Filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		SamplerShadowDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
		SamplerShadowDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		SamplerShadow = SamplerShadowDesc.CreateDescriptor();

		SamplerLinearClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		SamplerLinearClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		SamplerLinearClamp = SamplerLinearClampDesc.CreateDescriptor();

		SamplerVolumeWrapDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		SamplerVolumeWrap = SamplerVolumeWrapDesc.CreateDescriptor();

		SamplerPointClampDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		SamplerPointClampDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		SamplerPointClamp = SamplerPointClampDesc.CreateDescriptor();

		SamplerLinearBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		SamplerLinearBorderDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
		SamplerLinearBorderDesc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
		SamplerLinearBorder = SamplerLinearBorderDesc.CreateDescriptor();

		SamplerPointBorderDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
		SamplerPointBorderDesc.SetTextureAddressMode(D3D12_TEXTURE_ADDRESS_MODE_BORDER);
		SamplerPointBorderDesc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
		SamplerPointBorder = SamplerPointBorderDesc.CreateDescriptor();

		// Default rasterizer states
		RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
		RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;
		RasterizerDefault.FrontCounterClockwise = TRUE;
		RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDefault.DepthClipEnable = TRUE;
		RasterizerDefault.MultisampleEnable = FALSE;
		RasterizerDefault.AntialiasedLineEnable = FALSE;
		RasterizerDefault.ForcedSampleCount = 0;
		RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		RasterizerDefaultWireframe = RasterizerDefault;
		RasterizerDefaultWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

		RasterizerDefaultMsaa = RasterizerDefault;
		RasterizerDefaultMsaa.MultisampleEnable = TRUE;

		RasterizerDefaultMsaaWireframe = RasterizerDefaultMsaa;
		RasterizerDefaultMsaaWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

		RasterizerDefaultCw = RasterizerDefault;
		RasterizerDefaultCw.FrontCounterClockwise = FALSE;

		RasterizerDefaultCwWireframe = RasterizerDefaultCw;
		RasterizerDefaultCwWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

		RasterizerDefaultCwMsaa = RasterizerDefaultCw;
		RasterizerDefaultCwMsaa.MultisampleEnable = TRUE;

		RasterizerDefaultCwMsaaWireframe = RasterizerDefaultCwMsaa;
		RasterizerDefaultCwMsaaWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

		RasterizerTwoSided = RasterizerDefault;
		RasterizerTwoSided.CullMode = D3D12_CULL_MODE_NONE;

		RasterizerTwoSidedWireframe = RasterizerTwoSided;
		RasterizerTwoSidedWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

		RasterizerTwoSidedMsaa = RasterizerTwoSided;
		RasterizerTwoSidedMsaa.MultisampleEnable = TRUE;

		RasterizerTwoSidedMsaaWireframe = RasterizerTwoSidedMsaa;
		RasterizerTwoSidedMsaaWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

		// Shadows need their own rasterizer state so we can reverse the winding of faces
		RasterizerShadow = RasterizerDefault;
		//RasterizerShadow.CullMode = D3D12_CULL_FRONT;  // Hacked here rather than fixing the content
		RasterizerShadow.SlopeScaledDepthBias = -1.5f;
		RasterizerShadow.DepthBias = -50;

		RasterizerShadowTwoSided = RasterizerShadow;
		RasterizerShadowTwoSided.CullMode = D3D12_CULL_MODE_NONE;

		RasterizerShadowCW = RasterizerShadow;
		RasterizerShadowCW.FrontCounterClockwise = FALSE;

		RasterizerShadowWireframe = RasterizerShadow;
		RasterizerShadowWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

		RasterizerShadowCWWireframe = RasterizerShadowCW;
		RasterizerShadowCWWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

		RasterizerShadowTwoSidedWireframe = RasterizerShadowTwoSided;
		RasterizerShadowTwoSidedWireframe.FillMode = D3D12_FILL_MODE_WIREFRAME;

		DepthStateDisabled.DepthEnable = FALSE;
		DepthStateDisabled.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
		DepthStateDisabled.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		DepthStateDisabled.StencilEnable = FALSE;
		DepthStateDisabled.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
		DepthStateDisabled.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
		DepthStateDisabled.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
		DepthStateDisabled.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
		DepthStateDisabled.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
		DepthStateDisabled.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
		DepthStateDisabled.BackFace = DepthStateDisabled.FrontFace;

		DepthStateReadWrite = DepthStateDisabled;
		DepthStateReadWrite.DepthEnable = TRUE;
		DepthStateReadWrite.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		DepthStateReadWrite.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

		DepthStateReadOnly = DepthStateReadWrite;
		DepthStateReadOnly.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

		DepthStateReadOnlyReversed = DepthStateReadOnly;
		DepthStateReadOnlyReversed.DepthFunc = D3D12_COMPARISON_FUNC_LESS;

		DepthStateTestEqual = DepthStateReadOnly;
		DepthStateTestEqual.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;

		D3D12_BLEND_DESC alphaBlend = {};
		alphaBlend.IndependentBlendEnable = FALSE;
		alphaBlend.RenderTarget[0].BlendEnable = FALSE;
		alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		alphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		alphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		alphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		alphaBlend.RenderTarget[0].RenderTargetWriteMask = 0;
		BlendNoColorWrite = alphaBlend;

		alphaBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		BlendDisable = alphaBlend;

		alphaBlend.RenderTarget[0].BlendEnable = TRUE;
		BlendTraditional = alphaBlend;

		alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		BlendPreMultiplied = alphaBlend;

		alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
		BlendAdditive = alphaBlend;

		alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		BlendTraditionalAdditive = alphaBlend;

		DispatchIndirectCommandSignature[0].Dispatch();
		DispatchIndirectCommandSignature.Finalize();

		DrawIndirectCommandSignature[0].Draw();
		DrawIndirectCommandSignature.Finalize();

		CommonRS.Reset(4, 3);
		CommonRS[0].InitAsConstants(0, 4);
		CommonRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10);
		CommonRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 10);
		CommonRS[3].InitAsConstantBuffer(1);
		CommonRS.InitStaticSampler(0, SamplerLinearClampDesc);
		CommonRS.InitStaticSampler(1, SamplerPointBorderDesc);
		CommonRS.InitStaticSampler(2, SamplerLinearBorderDesc);
		CommonRS.Finalize(L"GraphicsCommonRS");

#define CreatePSO(ObjName, name ) \
		{ \
			ObjName.SetRootSignature(CommonRS); \
			auto shader = GetShaderByName(#name); \
			ObjName.SetComputeShader(shader->GetBufferPointer(), shader->GetBufferSize() ); \
			ObjName.Finalize(); \
		}

		CreatePSO(GenerateMipsLinearPSO[0], GenerateMipsLinearCS);
		CreatePSO(GenerateMipsLinearPSO[1], GenerateMipsLinearOddXCS);
		CreatePSO(GenerateMipsLinearPSO[2], GenerateMipsLinearOddYCS);
		CreatePSO(GenerateMipsLinearPSO[3], GenerateMipsLinearOddCS);
		CreatePSO(GenerateMipsGammaPSO[0], GenerateMipsGammaCS);
		CreatePSO(GenerateMipsGammaPSO[1], GenerateMipsGammaOddXCS);
		CreatePSO(GenerateMipsGammaPSO[2], GenerateMipsGammaOddYCS);
		CreatePSO(GenerateMipsGammaPSO[3], GenerateMipsGammaOddCS);

		/*DownsampleDepthPSO.SetRootSignature(g_CommonRS);
		DownsampleDepthPSO.SetRasterizerState(RasterizerTwoSided);
		DownsampleDepthPSO.SetBlendState(BlendDisable);
		DownsampleDepthPSO.SetDepthStencilState(DepthStateReadWrite);
		DownsampleDepthPSO.SetSampleMask(0xFFFFFFFF);
		DownsampleDepthPSO.SetInputLayout(0, nullptr);
		DownsampleDepthPSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		DownsampleDepthPSO.SetVertexShader(ScreenQuadCommonVS, sizeof(pScreenQuadCommonVS));
		DownsampleDepthPSO.SetPixelShader(DownsampleDepthPS, sizeof(g_pDownsampleDepthPS));
		DownsampleDepthPSO.SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
		DownsampleDepthPSO.Finalize();*/

#undef CreatePSO
	}

	void DestroyCommonStates()
	{
		/*for (uint32_t i = 0; i < kNumDefaultTextures; ++i)
			DefaultTextures[i].Destroy();*/

		DispatchIndirectCommandSignature.Destroy();
		DrawIndirectCommandSignature.Destroy();
	}

	void BuildShaders()
	{

		Shaders.push_back(ComPtr<ID3DBlob>());

		D_FILE::VisitFilesInDirectory(std::filesystem::current_path() / "Shaders", true, [&](Path const& path)
			{
				if (path.extension() != L".hlsl")
					return;

				auto shaderNameW = D_FILE::GetFileName(path);
				auto shaderName = WSTR2STR(shaderNameW);

				Device::ShaderCompatibilityCheck(D3D_SHADER_MODEL_6_2);

				std::wstring compiler;
				if (shaderName.ends_with("VS"))
					compiler = L"vs_6_2";
				else if (shaderName.ends_with("PS"))
					compiler = L"ps_6_2";
				else if (shaderName.ends_with("CS"))
					compiler = L"cs_6_2";
				else if (shaderName.ends_with("GS"))
					compiler = L"gs_6_2";
				else if (shaderName.ends_with("DS"))
					compiler = L"ds_6_2";
				else if (shaderName.ends_with("HS"))
					compiler = L"hs_6_2";
				else
					return;

				Shaders.push_back(CompileShader(path, L"main", compiler));
				ShaderNameMap[shaderName] = (UINT32)Shaders.size() - 1;
			});

	}

	void LoadDefaultResources()
	{
		// Creating default meshes
		{
			auto box = D_RENDERER_GEOMETRY_GENERATOR::CreateBox(1.f, 1.f, 1.f, 0);
			auto cylinder = D_RENDERER_GEOMETRY_GENERATOR::CreateCylinder(0.5f, 0.5f, 1, 40, 20);
			auto geosphere = D_RENDERER_GEOMETRY_GENERATOR::CreateGeosphere(0.5f, 40);
			auto gridp2 = D_RENDERER_GEOMETRY_GENERATOR::CreateGridQuadPatch(100.f, 100.f, 2, 2);
			auto gridp4 = D_RENDERER_GEOMETRY_GENERATOR::CreateGridQuadPatch(200.f, 200.f, 4, 4);
			auto gridp8 = D_RENDERER_GEOMETRY_GENERATOR::CreateGridQuadPatch(400.f, 400.f, 8, 8);
			auto gridp16 = D_RENDERER_GEOMETRY_GENERATOR::CreateGridQuadPatch(1000.f, 1000.f, 16, 16);
			auto grid100 = D_RENDERER_GEOMETRY_GENERATOR::CreateGrid(100.f, 100.f, 100, 100);
			auto quad = D_RENDERER_GEOMETRY_GENERATOR::CreateQuad(0.f, 0.f, 1.f, 1.f, 0.f);
			auto sphere = D_RENDERER_GEOMETRY_GENERATOR::CreateSphere(0.5f, 40, 40);
			auto lowSphere = D_RENDERER_GEOMETRY_GENERATOR::CreateSphere(0.5f, 10, 6);
			auto line = D_RENDERER_GEOMETRY_GENERATOR::CreateLine(0.f, 0.f, 0.f, 0.f, 0.f, -1.f);

			auto resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Box Mesh"), L"Box Mesh", L"Box Mesh", true);
			MultiPartMeshData<StaticMeshResource::VertexType> meshData;
			meshData.MeshData = box;
			auto res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::BoxMesh, { StaticMeshResource::GetResourceType(), res->GetId()} });

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Cylinder Mesh"), L"Cylinder Mesh", L"Cylinder Mesh", true);
			meshData.MeshData = cylinder;
			res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::CylinderMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Geosphere Mesh"), L"Geosphere Mesh", L"Geosphere Mesh", true);
			meshData.MeshData = geosphere;
			res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::GeosphereMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });
			
			{
				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 2x2 Mesh"), L"Grid 2x2 Mesh", L"Grid 2x2 Mesh", true);
				meshData.MeshData = gridp2;
				res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::GridPatch2x2Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 4x4 Mesh"), L"Grid 4x4 Mesh", L"Grid 4x4 Mesh", true);
				meshData.MeshData = gridp4;
				res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::GridPatch4x4Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 8x8 Mesh"), L"Grid 8x8 Mesh", L"Grid 8x8 Mesh", true);
				meshData.MeshData = gridp8;
				res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::GridPatch8x8Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 16x16 Mesh"), L"Grid 16x16 Mesh", L"Grid 16x16 Mesh", true);
				meshData.MeshData = gridp16;
				res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::GridPatch16x16Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

				resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Grid 100x100 Mesh"), L"Grid 100x100 Mesh", L"Grid 100x100 Mesh", true);
				meshData.MeshData = grid100;
				res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
				res->MakeGpuClean();
				res->MakeDiskClean();
				((StaticMeshResource*)res)->Create(meshData);
				DefaultResourceMap.insert({ DefaultResource::Grid100x100Mesh, { StaticMeshResource::GetResourceType(), res->GetId() } });
			}

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Quad Mesh"), L"Quad Mesh", L"Quad Mesh", true);
			meshData.MeshData = quad;
			res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::QuadMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Sphere Mesh"), L"Sphere Mesh", L"Sphere Mesh", true);
			meshData.MeshData = sphere;
			res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::SphereMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = D_RESOURCE::GetManager()->CreateResource<StaticMeshResource>(GenerateUuidFor("Low Poly Sphere Mesh"), L"Low Poly Sphere Mesh", L"Low Poly Sphere Mesh", true);
			meshData.MeshData = lowSphere;
			res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((StaticMeshResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::LowPolySphereMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = D_RESOURCE::GetManager()->CreateResource<BatchResource>(GenerateUuidFor("Line Mesh"), L"Line Mesh", L"Line Mesh", true);
			meshData.MeshData = line;
			res = D_RESOURCE::GetManager()->GetRawResource(resHandle);
			res->MakeGpuClean();
			res->MakeDiskClean();
			((BatchResource*)res)->Create(meshData);
			DefaultResourceMap.insert({ DefaultResource::LineMesh, { BatchResource::GetResourceType(), res->GetId() } });
		}

		// Create default textures
		{
#define CreateDefaultTexture2D(name, color) \
{ \
	auto defaultTextureHandle = D_RESOURCE::GetManager()->CreateResource<TextureResource>(GenerateUuidFor("Default Texture2D " #name), L"Default Texture2D " #name, L"Default Texture2D " #name, true); \
	auto textureRes = (TextureResource*)D_RESOURCE::GetManager()->GetRawResource(defaultTextureHandle); \
	textureRes->CreateRaw(color, DXGI_FORMAT_R8G8B8A8_UNORM, 4, 1, 1); \
	auto rRes = dynamic_cast<Resource*>(textureRes); \
	rRes->MakeGpuClean(); \
	rRes->MakeDiskClean(); \
	DefaultResourceMap.insert({ DefaultResource::Texture2D##name, { TextureResource::GetResourceType(), textureRes->GetId() } }); \
}

#define CreateDefaultTextureCubeMap(name, color) \
{ \
	auto defaultTextureHandle = D_RESOURCE::GetManager()->CreateResource<TextureResource>(GenerateUuidFor("Default TextureCubeMap" #name), L"Default Texture2D " #name, L"Default Texture2D " #name, true); \
	auto textureRes = (TextureResource*)D_RESOURCE::GetManager()->GetRawResource(defaultTextureHandle); \
	textureRes->CreateCubeMap(color, DXGI_FORMAT_R8G8B8A8_UNORM, 4, 1, 1); \
	auto rRes = dynamic_cast<Resource*>(textureRes); \
	rRes->MakeGpuClean(); \
	rRes->MakeDiskClean(); \
	DefaultResourceMap.insert({ DefaultResource::TextureCubeMap##name, { TextureResource::GetResourceType(), textureRes->GetId() } }); \
}

			CreateDefaultTexture2D(Magenta, 0xFFFF00FF);
			CreateDefaultTexture2D(BlackOpaque, 0xFF000000);
			CreateDefaultTexture2D(BlackTransparent, 0x00000000);
			CreateDefaultTexture2D(WhiteOpaque, 0xFFFFFFFF);
			CreateDefaultTexture2D(WhiteTransparent, 0x00FFFFFF);
			CreateDefaultTexture2D(NormalMap, 0x00FF0000);

			uint32_t blackCubeTexels[6] = {};
			CreateDefaultTextureCubeMap(Black, blackCubeTexels);
		}

		// Creating default materials
		{
			auto defaultMaterialHandle = D_RESOURCE::GetManager()->CreateResource<MaterialResource>(GenerateUuidFor("Default Material"), L"Default Material", L"Default Material", true);
			auto materialRes = (MaterialResource*)D_RESOURCE::GetManager()->GetRawResource(defaultMaterialHandle);
			auto mat = materialRes->ModifyMaterialData();
			mat->DifuseAlbedo = XMFLOAT4(Vector4(kOne));
			mat->FresnelR0 = XMFLOAT3(Vector3(kOne) * 0.56f);
			mat->Metallic = 0.f;
			mat->Roughness = 0.f;
			auto rRes = dynamic_cast<Resource*>(materialRes);
			DefaultResourceMap.insert({ DefaultResource::Material, { MaterialResource::GetResourceType(), materialRes->GetId() } });
		}

	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		if (ImGui::CollapsingHeader("Temporal Anti-Aliasing"))
		{
			settingsChanged |= D_GRAPHICS_AA_TEMPORAL::OptionsDrawer(options);
		}

		if (ImGui::CollapsingHeader("Post Processing"))
		{
			settingsChanged |= D_GRAPHICS_PP::OptionsDrawer(options);
		}

		if (ImGui::CollapsingHeader("SSAO"))
		{
			settingsChanged |= D_GRAPHICS_AO_SS::OptionsDrawer(options);
		}

		if (ImGui::CollapsingHeader("Renderer"))
		{
			settingsChanged |= D_RENDERER::OptionsDrawer(options);
		}

		if (ImGui::CollapsingHeader("Info"))
		{
			ImGui::BeginGroup();

			if (ImGui::CollapsingHeader("Active Pipelines"))
			{
				auto& psos = D_RENDERER::GetPsos();

				ImGui::Text("Size: ");
				ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Text, { 0.f, 1.f, 0.f, 1.f });
				ImGui::Text(std::to_string(psos.size()).c_str());
				ImGui::PopStyleColor();

				auto flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Hideable;

				if (ImGui::BeginTable("Psos", 2, flags))
				{
					ImGui::TableSetupScrollFreeze(0, 1);
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Name");
					ImGui::TableHeadersRow();

					ImGuiListClipper clipper;
					clipper.Begin(200);
					while (clipper.Step())
					{
						for (int row = clipper.DisplayStart; row < (int)psos.size() && row < clipper.DisplayEnd; row++)
						{
							auto& pso = psos[row];

							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							ImGui::Text(std::to_string(row).c_str());
							ImGui::TableSetColumnIndex(1);
							auto name = pso.GetName();
							ImGui::Text(WSTR2STR(name).c_str());
						}
					}

					ImGui::EndTable();
				}
			}

			ImGui::EndGroup();

		}

		D_H_OPTION_DRAW_END()

	}
#endif

	Microsoft::WRL::ComPtr<ID3DBlob>		GetShaderByIndex(UINT32 index)
	{
		return Shaders[index];
	}

	Microsoft::WRL::ComPtr<ID3DBlob>		GetShaderByName(std::string const& shaderName)
	{
		return Shaders[ShaderNameMap[shaderName]];
	}

	UINT32									GetShaderIndex(std::string const& shaderName)
	{
		return ShaderNameMap[shaderName];
	}
}