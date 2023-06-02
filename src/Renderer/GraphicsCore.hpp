#pragma once

#include "CommandSignature.hpp"
#include "GraphicsUtils/CommandListManager.hpp"
#include "GraphicsUtils/PipelineState.hpp"
#include "GraphicsUtils/RootSignature.hpp"
#include "GraphicsUtils/SamplerManager.hpp"
#include "GraphicsDeviceManager.hpp"

#include <Core/Serialization/Json.hpp>
#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceManager.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif // !D_GRAPHICS

namespace Darius::Graphics
{
	class ContextManager;

	void									Initialize(HWND window, int width, int height, D_SERIALIZATION::Json const& settings);
	void									Shutdown();

	void									Present();

	uint32_t								GetFrameCount();

	DXGI_FORMAT								GetColorFormat();
	DXGI_FORMAT								GetDepthFormat();
	DXGI_FORMAT								GetShadowFormat();
	DXGI_FORMAT								SwapChainGetColorFormat();
	DXGI_FORMAT								SwapChainGetDepthFormat();

	HWND									GetWindow();

	bool									SupportsTypedUAVLoadSupport_R11G11B10_FLOAT();
	bool									SupportsTypedUAVLoadSupport_R16G16B16A16_FLOAT();

	D3D12_CPU_DESCRIPTOR_HANDLE				AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1);

	D_GRAPHICS_UTILS::CommandListManager*	GetCommandManager();
	ContextManager*							GetContextManager();

	Microsoft::WRL::ComPtr<ID3DBlob>		GetShaderByIndex(UINT32 index);
	Microsoft::WRL::ComPtr<ID3DBlob>		GetShaderByName(std::string const& shaderName);
	UINT32									GetShaderIndex(std::string const& shaderName);


#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif


	enum class DefaultResource
	{
		// Meshes
		BoxMesh,
		CylinderMesh,
		GeosphereMesh,
		QuadMesh,
		SphereMesh,
		LowPolySphereMesh,
		LineMesh,
		GridPatch2x2Mesh,
		GridPatch4x4Mesh,
		GridPatch8x8Mesh,
		GridPatch16x16Mesh,
		Grid100x100Mesh,

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
	extern D3D12_RASTERIZER_DESC			RasterizerShadowWireframe;
	extern D3D12_RASTERIZER_DESC			RasterizerShadowCWWireframe;
	extern D3D12_RASTERIZER_DESC			RasterizerShadowTwoSidedWireframe;

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