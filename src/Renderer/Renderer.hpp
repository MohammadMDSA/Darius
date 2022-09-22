#pragma once

#include "RenderDeviceManager.hpp"
#include "CommandContext.hpp"

#include <Core/Containers/Vector.hpp>
#include <Math/Transform.hpp>

#define D_RENDERER Darius::Renderer
#define D_DEVICE D_RENDERER_DEVICE

using namespace Darius::Math;
using namespace Darius::Renderer::DeviceResource;
using namespace Microsoft::WRL;

namespace Darius::Renderer
{
	extern UINT PassCbvOffset;

	enum RootBindings
	{
		kMeshConstants,			// Holds mesh constants only in VS
		kMaterialConstants,		// Holds material constants only in PS
		kMaterialSRVs,
		kMaterialSamplers,
		kCommonCBV,				// Holds global constants in all shaders
		kCommonSRVs,			

		kNumRootBindings		// Just to know how many root binings there are
	};

	enum ColorRootBindings
	{
		kColorMeshConstants,
		kColorConstants,
		kColorCommonCBV,

		kColorNumRootBindings
	};

	enum TextureType
	{
		kBaseColor,
		kRoughness,
		kOcculusion,
		kEmissive,
		kNormal,

		kNumTextures
	};

	enum class PipelineStateTypes
	{
		Opaque,
		Wireframe,
		Color,

		_num
	};

	enum class RootSignatureTypes
	{
		Default,
		Color,

		_num
	};

	void Initialize();
	void Shutdown();

#ifdef _D_EDITOR
	DescriptorHandle		GetUiTextureHandle(UINT index);
	void					RenderGui(D_GRAPHICS::GraphicsContext& context);
#endif
	// For rendering meshs
	void					RenderMeshes(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems, D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals);

	// For rendering batches like line and debug stuff
	void					RenderBatchs(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems, D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals);

	void					Present(D_GRAPHICS::GraphicsContext& context);

	D_GRAPHICS_UTILS::GraphicsPSO& GetPSO(PipelineStateTypes type);
	D_GRAPHICS_UTILS::RootSignature& GetRootSignature(RootSignatureTypes type);

	DescriptorHandle		AllocateTextureDescriptor(UINT count = 1);
}