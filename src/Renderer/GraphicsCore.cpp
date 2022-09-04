#include "pch.hpp"
#include "GraphicsCore.hpp"
#include "GraphicsUtils/Memory/DescriptorHeap.hpp"
#include "Renderer.hpp"
#include "RenderDeviceManager.hpp"
#include "CommandContext.hpp"
#include "GraphicsUtils/D3DUtils.hpp"
#include "GraphicsUtils/Profiling/GpuTimeManager.hpp"
#include "LightManager.hpp"
#include "GraphicsUtils/Buffers/Texture.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Utils/Assert.hpp>

using namespace D_GRAPHICS_MEMORY;
using namespace D_GRAPHICS_BUFFERS;

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

	std::unordered_map<std::string, ComPtr<ID3DBlob>>	Shaders;

	//////////////////////////////////
	////////// Common States /////////
	//////////////////////////////////

	SamplerDesc SamplerLinearWrapDesc;
	SamplerDesc SamplerAnisoWrapDesc;
	SamplerDesc SamplerShadowDesc;
	SamplerDesc SamplerLinearClampDesc;
	SamplerDesc SamplerVolumeWrapDesc;
	SamplerDesc SamplerPointClampDesc;
	SamplerDesc SamplerPointBorderDesc;
	SamplerDesc SamplerLinearBorderDesc;

	D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerAnisoWrap;
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadow;
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearClamp;
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerVolumeWrap;
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointClamp;
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointBorder;
	D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearBorder;

	D3D12_RASTERIZER_DESC RasterizerDefault;	// Counter-clockwise
	D3D12_RASTERIZER_DESC RasterizerDefaultMsaa;
	D3D12_RASTERIZER_DESC RasterizerDefaultCw;	// Clockwise winding
	D3D12_RASTERIZER_DESC RasterizerDefaultCwMsaa;
	D3D12_RASTERIZER_DESC RasterizerTwoSided;
	D3D12_RASTERIZER_DESC RasterizerTwoSidedMsaa;
	D3D12_RASTERIZER_DESC RasterizerShadow;
	D3D12_RASTERIZER_DESC RasterizerShadowCW;
	D3D12_RASTERIZER_DESC RasterizerShadowTwoSided;

	D3D12_BLEND_DESC BlendNoColorWrite;
	D3D12_BLEND_DESC BlendDisable;
	D3D12_BLEND_DESC BlendPreMultiplied;
	D3D12_BLEND_DESC BlendTraditional;
	D3D12_BLEND_DESC BlendAdditive;
	D3D12_BLEND_DESC BlendTraditionalAdditive;

	D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;
	D3D12_DEPTH_STENCIL_DESC DepthStateReadWrite;
	D3D12_DEPTH_STENCIL_DESC DepthStateReadOnly;
	D3D12_DEPTH_STENCIL_DESC DepthStateReadOnlyReversed;
	D3D12_DEPTH_STENCIL_DESC DepthStateTestEqual;

	CommandSignature					DispatchIndirectCommandSignature(1);
	CommandSignature					DrawIndirectCommandSignature(1);

	D_GRAPHICS_UTILS::RootSignature		CommonRS;
	/*ComputePSO GenerateMipsLinearPSO[4] =
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
	};*/

	/*GraphicsPSO DownsampleDepthPSO(L"DownsampleDepth PSO")*/;

	void InitializeCommonStates();
	void DestroyCommonStates();
	void BuildShaders();

	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;
		CommandManager.Create(D_RENDERER_DEVICE::GetDevice());

		DispatchIndirectCommandSignature[0].Dispatch();
		DispatchIndirectCommandSignature.Finalize();

		DrawIndirectCommandSignature[0].Draw();
		DrawIndirectCommandSignature.Finalize();

		D_PROFILING_GPU::Initialize(4096);

		InitializeCommonStates();
		BuildShaders();

		D_LIGHT::Initialize();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		D_LIGHT::Shutdown();

		D_PROFILING_GPU::Shutdown();

		CommandManager.IdleGPU();
		CommandContext::DestroyAllContexts();
		CommandManager.Shutdown();

		DescriptorAllocator::DestroyAll();

		PSO::DestroyAll();

		D_GRAPHICS_UTILS::RootSignature::DestroyAll();
		DescriptorAllocator::DestroyAll();

		DestroyCommonStates();

		for (auto& kv : Shaders)
			kv.second.Reset();

	}

	uint32_t GetFrameCount()
	{
		return D_TIME::GetFrameCount();
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
		RasterizerDefault.AntialiasedLineEnable = TRUE;
		RasterizerDefault.ForcedSampleCount = 0;
		RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		RasterizerDefaultMsaa = RasterizerDefault;
		RasterizerDefaultMsaa.MultisampleEnable = TRUE;

		RasterizerDefaultCw = RasterizerDefault;
		RasterizerDefaultCw.FrontCounterClockwise = FALSE;

		RasterizerDefaultCwMsaa = RasterizerDefaultCw;
		RasterizerDefaultCwMsaa.MultisampleEnable = TRUE;

		RasterizerTwoSided = RasterizerDefault;
		RasterizerTwoSided.CullMode = D3D12_CULL_MODE_NONE;

		RasterizerTwoSidedMsaa = RasterizerTwoSided;
		RasterizerTwoSidedMsaa.MultisampleEnable = TRUE;

		// Shadows need their own rasterizer state so we can reverse the winding of faces
		RasterizerShadow = RasterizerDefault;
		//RasterizerShadow.CullMode = D3D12_CULL_FRONT;  // Hacked here rather than fixing the content
		RasterizerShadow.SlopeScaledDepthBias = -1.5f;
		RasterizerShadow.DepthBias = -100;

		RasterizerShadowTwoSided = RasterizerShadow;
		RasterizerShadowTwoSided.CullMode = D3D12_CULL_MODE_NONE;

		RasterizerShadowCW = RasterizerShadow;
		RasterizerShadowCW.FrontCounterClockwise = FALSE;

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

#define CreatePSO(ObjName, ShaderByteCode ) \
		ObjName.SetRootSignature(g_CommonRS); \
		ObjName.SetComputeShader(ShaderByteCode, sizeof(ShaderByteCode) ); \
		ObjName.Finalize();

		/*CreatePSO(GenerateMipsLinearPSO[0], g_pGenerateMipsLinearCS);
		CreatePSO(GenerateMipsLinearPSO[1], g_pGenerateMipsLinearOddXCS);
		CreatePSO(GenerateMipsLinearPSO[2], g_pGenerateMipsLinearOddYCS);
		CreatePSO(GenerateMipsLinearPSO[3], g_pGenerateMipsLinearOddCS);
		CreatePSO(GenerateMipsGammaPSO[0], g_pGenerateMipsGammaCS);
		CreatePSO(GenerateMipsGammaPSO[1], g_pGenerateMipsGammaOddXCS);
		CreatePSO(GenerateMipsGammaPSO[2], g_pGenerateMipsGammaOddYCS);
		CreatePSO(GenerateMipsGammaPSO[3], g_pGenerateMipsGammaOddCS);*/

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

		Shaders["standardVS"] = CompileShader(L"Shaders\\DefaultVS.hlsl", nullptr, "main", "vs_5_1");
		Shaders["opaquePS"] = CompileShader(L"Shaders\\DefaultPS.hlsl", nullptr, "main", "ps_5_1");
		Shaders["colorVS"] = CompileShader(L"Shaders\\SimpleColorVS.hlsl", nullptr, "main", "vs_5_1");
		Shaders["colorPS"] = CompileShader(L"Shaders\\SimpleColorPS.hlsl", nullptr, "main", "ps_5_1");

	}

}