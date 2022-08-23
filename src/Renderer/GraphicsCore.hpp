#pragma once

#include "GraphicsUtils/CommandListManager.hpp"
#include "CommandSignature.hpp"
#include "GraphicsUtils/SamplerManager.hpp"
#include "GraphicsUtils/RootSignature.hpp"
#include "GraphicsUtils/PipelineState.hpp"

#define D_GRAPHICS Darius::Graphics

using namespace D_GRAPHICS_UTILS;
using namespace Microsoft::WRL;

namespace Darius::Graphics
{
	class ContextManager;

	void Initialize();
	void Shutdown();

	void Tick();

	uint64_t GetFrameCount();

	D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1);

	CommandListManager* GetCommandManager();
	ContextManager*		GetContextManager();

	//D3D12_CPU_DESCRIPTOR_HANDLE GetDefaultTexture(eDefaultTexture texID);
	
	extern std::unordered_map<std::string, ComPtr<ID3DBlob>>	Shaders;

	extern SamplerDesc SamplerLinearWrapDesc;
	extern SamplerDesc SamplerAnisoWrapDesc;
	extern SamplerDesc SamplerShadowDesc;
	extern SamplerDesc SamplerLinearClampDesc;
	extern SamplerDesc SamplerVolumeWrapDesc;
	extern SamplerDesc SamplerPointClampDesc;
	extern SamplerDesc SamplerPointBorderDesc;
	extern SamplerDesc SamplerLinearBorderDesc;

	extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearWrap;
	extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerAnisoWrap;
	extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerShadow;
	extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearClamp;
	extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerVolumeWrap;
	extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointClamp;
	extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerPointBorder;
	extern D3D12_CPU_DESCRIPTOR_HANDLE SamplerLinearBorder;

	extern D3D12_RASTERIZER_DESC RasterizerDefault;	// Counter-clockwise
	extern D3D12_RASTERIZER_DESC RasterizerDefaultMsaa;
	extern D3D12_RASTERIZER_DESC RasterizerDefaultCw;	// Clockwise winding
	extern D3D12_RASTERIZER_DESC RasterizerDefaultCwMsaa;
	extern D3D12_RASTERIZER_DESC RasterizerTwoSided;
	extern D3D12_RASTERIZER_DESC RasterizerTwoSidedMsaa;
	extern D3D12_RASTERIZER_DESC RasterizerShadow;
	extern D3D12_RASTERIZER_DESC RasterizerShadowCW;
	extern D3D12_RASTERIZER_DESC RasterizerShadowTwoSided;

	extern D3D12_BLEND_DESC BlendNoColorWrite;
	extern D3D12_BLEND_DESC BlendDisable;
	extern D3D12_BLEND_DESC BlendPreMultiplied;
	extern D3D12_BLEND_DESC BlendTraditional;
	extern D3D12_BLEND_DESC BlendAdditive;
	extern D3D12_BLEND_DESC BlendTraditionalAdditive;

	extern D3D12_DEPTH_STENCIL_DESC DepthStateDisabled;
	extern D3D12_DEPTH_STENCIL_DESC DepthStateReadWrite;
	extern D3D12_DEPTH_STENCIL_DESC DepthStateReadOnly;
	extern D3D12_DEPTH_STENCIL_DESC DepthStateReadOnlyReversed;
	extern D3D12_DEPTH_STENCIL_DESC DepthStateTestEqual;

	extern CommandSignature					DispatchIndirectCommandSignature;
	extern CommandSignature					DrawIndirectCommandSignature;

	extern D_GRAPHICS_UTILS::RootSignature	CommonRS;

	enum eDefaultTexture
	{
		kMagenta2D,  // Useful for indicating missing textures
		kBlackOpaque2D,
		kBlackTransparent2D,
		kWhiteOpaque2D,
		kWhiteTransparent2D,
		kDefaultNormalMap,
		kBlackCubeMap,

		kNumDefaultTextures
	};
}