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
		kCommonCBV,				// Holds global constants in all shaders

		kNumRootBindings		// Just to know how many root binings there are
	};

	void Initialize();
	void Shutdown();

	extern std::unordered_map<std::string, D_GRAPHICS_UTILS::GraphicsPSO> Psos;
	extern D_GRAPHICS_UTILS::RootSignature RootSign;

#ifdef _D_EDITOR
	void RegisterGuiDrawer(std::function<void(void)> drawer);
	DescriptorHandle GetRenderResourceHandle(UINT index);
#endif
	void RenderMeshes(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems, D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals);

	void Present(D_GRAPHICS::GraphicsContext& context);

}