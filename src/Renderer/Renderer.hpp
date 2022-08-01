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
	extern D_GRAPHICS_UTILS::RootSignature RootSig;
	extern std::unordered_map<std::string, D_GRAPHICS_UTILS::GraphicsPSO> Psos;
	extern UINT PassCbvOffset;
	extern std::unordered_map<std::string, ComPtr<ID3DBlob>> Shaders;
	extern std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

	// For now, just a simple cube draw
	// Definitely bad input signature but we will get there...
	void Initialize();
	void Shutdown();

#ifdef _D_EDITOR
	void RegisterGuiDrawer(std::function<void(void)> drawer);
	DescriptorHandle GetRenderResourceHandle(UINT index);
#endif
	void RenderMeshes(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems, D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals);

	void UpdateMeshCBs(D_CONTAINERS::DVector<RenderItem> const& renderItems);

	void Present(D_GRAPHICS::GraphicsContext& context);

}