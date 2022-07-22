#pragma once

#include <Math/Transform.hpp>
#include "DeviceResources.hpp"

#define D_RENDERER Darius::Renderer
#define D_RENDERER_DEVICE Darius::Renderer::Device
#define D_DEVICE D_RENDERER_DEVICE

using namespace Darius::Math;
using namespace Darius::Renderer::DeviceResource;
using namespace Microsoft::WRL;

namespace Darius::Renderer
{
	// For now, just a simple cube draw
	// Definitely bad input signature but we will get there...
	void Initialize();
	void Shutdown();

	void RenderMeshes(std::vector<RenderItem*> const& renderItems);
	void UpdateMeshCBs(std::vector<RenderItem*> const& renderItems);

	namespace Device
	{
		void RegisterDeviceNotify(IDeviceNotify* notify);
		void Initialize(HWND window, int width, int height);
		void Shutdown();

		extern ComPtr<ID3D12DescriptorHeap> CbvHeap;
		extern ComPtr<ID3D12RootSignature> RootSignature;
		extern std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> Psos;
		extern UINT PassCbvOffset;
		extern std::unordered_map<std::string, ComPtr<ID3DBlob>> Shaders;
		extern std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;

		FrameResource* GetCurrentFrameResource();
		ID3D12Device* GetDevice();
		ID3D12GraphicsCommandList* GetCommandList();
		ID3D12CommandAllocator* GetCommandAllocator();
		ID3D12CommandQueue* GetCommandQueue();
		FrameResource* GetFrameResourceWithIndex(int i);
		DXGI_FORMAT GetBackBufferFormat();
		DXGI_FORMAT GetDepthBufferFormat();
		void WaitForGpu();

		// Window functions
		void OnWindowMoved();
		void OnDisplayChanged();
		bool OnWindowsSizeChanged(int width, int height);
		void ShaderCompatibilityCheck(D3D_SHADER_MODEL shaderModel);

		void SyncFrame();
	}
}