#pragma once

#include "RenderDeviceManager.hpp"

#include <Math/Transform.hpp>

#define D_RENDERER Darius::Renderer
#define D_DEVICE D_RENDERER_DEVICE

using namespace Darius::Math;
using namespace Darius::Renderer::DeviceResource;
using namespace Microsoft::WRL;

namespace Darius::Renderer
{
	extern ComPtr<ID3D12RootSignature> RootSignature;
	extern std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> Psos;
	extern UINT PassCbvOffset;
	extern std::unordered_map<std::string, ComPtr<ID3DBlob>> Shaders;
	extern std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

	// For now, just a simple cube draw
	// Definitely bad input signature but we will get there...
	void Initialize();
	void Shutdown();

#ifdef _D_EDITOR
	void RegisterGuiDrawer(std::function<void(void)> drawer);
#endif
	void RenderMeshes(std::vector<RenderItem*> const& renderItems);
	void UpdateMeshCBs(std::vector<RenderItem*> const& renderItems);

	//size_t GetSceneTextureHandle();
}