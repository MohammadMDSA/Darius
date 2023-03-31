#pragma once

#include "CommandSignature.hpp"
#include "GraphicsUtils/CommandListManager.hpp"
#include "GraphicsUtils/PipelineState.hpp"
#include "GraphicsUtils/RootSignature.hpp"
#include "GraphicsUtils/SamplerManager.hpp"

#include <Core/Serialization/Json.hpp>
#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceManager.hpp>

#define D_GRAPHICS Darius::Graphics

namespace Darius::Graphics
{
	class ContextManager;

	void									Initialize(D_SERIALIZATION::Json const& settings);
	void									Shutdown();

	uint32_t								GetFrameCount();
	uint32_t								ProceedFrame();

	D3D12_CPU_DESCRIPTOR_HANDLE				AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1);

	D_GRAPHICS_UTILS::CommandListManager*	GetCommandManager();
	ContextManager*							GetContextManager();

	enum class DefaultResource
	{
		// Meshes
		BoxMesh,
		CylinderMesh,
		GeosphereMesh,
		GridMesh,
		QuadMesh,
		SphereMesh,
		LowPolySphereMesh,
		LineMesh,

		// Materials
		Material,

		// Textures
		Texture2DMagenta,
		Texture2DBlackOpaque,
		Texture2DBlackTransparent,
		Texture2DWhiteOpaque,
		Texture2DWhiteTransparent,
		Texture2DNormalMap,
		TextureCubeMapBlack
	};

	D_RESOURCE::ResourceHandle GetDefaultGraphicsResource(DefaultResource type);

	extern std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>>	Shaders;

	extern D_GRAPHICS_UTILS::SamplerDesc	SamplerLinearWrapDesc;
	extern D_GRAPHICS_UTILS::SamplerDesc	SamplerAnisoWrapDesc;
	extern D_GRAPHICS_UTILS::SamplerDesc	SamplerShadowDesc;
	extern D_GRAPHICS_UTILS::SamplerDesc	SamplerLinearClampDesc;
	extern D_GRAPHICS_UTILS::SamplerDesc	SamplerVolumeWrapDesc;
	extern D_GRAPHICS_UTILS::SamplerDesc	SamplerPointClampDesc;
	extern D_GRAPHICS_UTILS::SamplerDesc	SamplerPointBorderDesc;
	extern D_GRAPHICS_UTILS::SamplerDesc	SamplerLinearBorderDesc;

	extern D3D12_CPU_DESCRIPTOR_HANDLE		SamplerLinearWrap;
	extern D3D12_CPU_DESCRIPTOR_HANDLE		SamplerAnisoWrap;
	extern D3D12_CPU_DESCRIPTOR_HANDLE		SamplerShadow;
	extern D3D12_CPU_DESCRIPTOR_HANDLE		SamplerLinearClamp;
	extern D3D12_CPU_DESCRIPTOR_HANDLE		SamplerVolumeWrap;
	extern D3D12_CPU_DESCRIPTOR_HANDLE		SamplerPointClamp;
	extern D3D12_CPU_DESCRIPTOR_HANDLE		SamplerPointBorder;
	extern D3D12_CPU_DESCRIPTOR_HANDLE		SamplerLinearBorder;

	extern D3D12_RASTERIZER_DESC			RasterizerDefault;	// Counter-clockwise
	extern D3D12_RASTERIZER_DESC			RasterizerDefaultWireframe;
	extern D3D12_RASTERIZER_DESC			RasterizerDefaultMsaa;
	extern D3D12_RASTERIZER_DESC			RasterizerDefaultMsaaWireframe;
	extern D3D12_RASTERIZER_DESC			RasterizerDefaultCw;	// Clockwise winding
	extern D3D12_RASTERIZER_DESC			RasterizerDefaultCwWireframe;
	extern D3D12_RASTERIZER_DESC			RasterizerDefaultCwMsaa;
	extern D3D12_RASTERIZER_DESC			RasterizerDefaultCwMsaaWireframe;
	extern D3D12_RASTERIZER_DESC			RasterizerTwoSided;
	extern D3D12_RASTERIZER_DESC			RasterizerTwoSidedWireframe;
	extern D3D12_RASTERIZER_DESC			RasterizerTwoSidedMsaa;
	extern D3D12_RASTERIZER_DESC			RasterizerTwoSidedMsaaWireframe;
	extern D3D12_RASTERIZER_DESC			RasterizerShadow;
	extern D3D12_RASTERIZER_DESC			RasterizerShadowCW;
	extern D3D12_RASTERIZER_DESC			RasterizerShadowTwoSided;

	extern D3D12_BLEND_DESC					BlendNoColorWrite;
	extern D3D12_BLEND_DESC					BlendDisable;
	extern D3D12_BLEND_DESC					BlendPreMultiplied;
	extern D3D12_BLEND_DESC					BlendTraditional;
	extern D3D12_BLEND_DESC					BlendAdditive;
	extern D3D12_BLEND_DESC					BlendTraditionalAdditive;

	extern D3D12_DEPTH_STENCIL_DESC			DepthStateDisabled;
	extern D3D12_DEPTH_STENCIL_DESC			DepthStateReadWrite;
	extern D3D12_DEPTH_STENCIL_DESC			DepthStateReadOnly;
	extern D3D12_DEPTH_STENCIL_DESC			DepthStateReadOnlyReversed;
	extern D3D12_DEPTH_STENCIL_DESC			DepthStateTestEqual;

	extern CommandSignature					DispatchIndirectCommandSignature;
	extern CommandSignature					DrawIndirectCommandSignature;

	extern D_GRAPHICS_UTILS::RootSignature	CommonRS;
	extern D_GRAPHICS_UTILS::ComputePSO		GenerateMipsLinearPSO[4];
	extern D_GRAPHICS_UTILS::ComputePSO		GenerateMipsGammaPSO[4];

}