#include "Renderer.hpp"
#include "Renderer.hpp"
#include "pch.hpp"
#include "Renderer.hpp"
#include "Geometry/Mesh.hpp"
#include "FrameResource.hpp"
#include "GraphicsCore.hpp"
#include "GraphicsUtils/D3DUtils.hpp"
#include "GraphicsUtils/UploadBuffer.hpp"
#include "GraphicsUtils/RootSignature.hpp"
#include "GraphicsUtils/Memory/DescriptorHeap.hpp"
#include "GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "Camera/CameraManager.hpp"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include <Core/TimeManager/TimeManager.hpp>
#include <Math/VectorMath.hpp>
#include <Utils/Assert.hpp>

#include <filesystem>


using namespace Microsoft::WRL;

using namespace Darius::Math;
using namespace Darius::Renderer::DeviceResource;
using namespace Darius::Renderer::GraphicsUtils;
using namespace D_RENDERER_GEOMETRY;
using namespace D_GRAPHICS_MEMORY;
using namespace D_GRAPHICS_UTILS;

namespace Darius::Renderer
{

	ID3D12Device* _device = nullptr;

	// Input layout and root signature
	std::vector<D3D12_INPUT_ELEMENT_DESC>				InputLayout;
	D_GRAPHICS_UTILS::RootSignature						RootSig;
	DescriptorHeap										DrawHeap;

	// Shaders
	std::unordered_map<std::string, ComPtr<ID3DBlob>>	Shaders;

#ifdef _D_EDITOR
	std::function<void(void)>							GuiDrawer = nullptr;
	DescriptorHeap										ImguiHeap;
	UINT												MaxImguiElements = 2;
#endif

	// PSO
	std::unordered_map<std::string, D_GRAPHICS_UTILS::GraphicsPSO> Psos;

	// Device resource
	std::unique_ptr<DeviceResource::DeviceResources>	Resources;

	///////////////// REMOVE ASAP //////////////////
	bool _4xMsaaState = false;
	short _4xMsaaQuality = 0;

	//////////////////////////////////////////////////////
	// Functions

#ifdef _D_EDITOR
	void InitializeGUI();
#endif

	void DrawCube(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems);

	void UpdateGlobalConstants(D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals);

	void Clear(D_GRAPHICS::GraphicsContext& context, D_GRAPHICS_BUFFERS::ColorBuffer& rt, D_GRAPHICS_BUFFERS::DepthBuffer& depthStencil, RECT bounds, std::wstring const& processName = L"Clear");

	void Initialize()
	{
		D_ASSERT(_device == nullptr);
		D_ASSERT(Resources);

		_device = Resources->GetD3DDevice();

		DrawHeap.Create(L"Draw Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100010 * D_RENDERER_FRAME_RESOUCE::gNumFrameResources);

		// Create Constant Buffer Views
		UINT objSize = D_RENDERER_UTILS::CalcConstantBufferByteSize(sizeof(MeshConstants));

		UINT objCount = 100010;
		for (size_t frameResIdx = 0; frameResIdx < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; frameResIdx++)
		{
			auto meshCB = Resources->GetFrameResourceByIndex(frameResIdx)->MeshCB->Resource();
			for (UINT i = 0; i < objCount; i++)
			{
				D3D12_GPU_VIRTUAL_ADDRESS cbAddress = meshCB->GetGPUVirtualAddress();

				// Offset to the ith object constant buffer in the buffer.
				cbAddress += i * objSize;
				// Offset to the object cbv in the descriptor heap.
				int heapIndex = frameResIdx * objCount + i;
				auto handle = DrawHeap.Alloc();

				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
				cbvDesc.BufferLocation = cbAddress;
				cbvDesc.SizeInBytes = objSize;

				_device->CreateConstantBufferView(&cbvDesc, handle);
			}
		}


#ifdef _D_EDITOR
		InitializeGUI();
#endif // _D_EDITOR

	}

	void Shutdown()
	{
		D_ASSERT(_device != nullptr);

	}

#ifdef _D_EDITOR
	void RegisterGuiDrawer(std::function<void(void)> drawer)
	{
		GuiDrawer = drawer;
	}
#endif

	void RenderMeshes(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems, D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals)
	{
		UpdateGlobalConstants(globals);

		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, L"Render opaque");

		DrawCube(context, renderItems);

		PIXEndEvent(context.GetCommandList());
	}

	void Present(D_GRAPHICS::GraphicsContext& context)
	{

#ifdef _D_EDITOR
		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, L"Render gui");
		if (GuiDrawer)
		{
			auto& viewportRt = Resources->GetRTBuffer();
			context.TransitionResource(viewportRt, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

			// Clear RT
			Clear(context, Resources->GetRTBuffer(), Resources->GetDepthStencilBuffer(), Resources->GetOutputSize());

			// Prepare imgui
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			GuiDrawer();

			ImGui::Render();
			context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ImguiHeap.GetHeapPointer());
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());
		}
		PIXEndEvent(context.GetCommandList());
#endif

		context.Flush();

		// Show the new frame.
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
		Resources->Present(context);

		// If using the DirectX Tool Kit for DX12, uncomment this line:
		// m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

		PIXEndEvent();
	}

	void UpdateMeshCBs(D_CONTAINERS::DVector<RenderItem> const& renderItems)
	{
		PIXBeginEvent(PIX_COLOR_INDEX(10), "Update Mesh CBs");
		auto frameResource = Resources->GetFrameResource();
		frameResource->ReinitializeMeshCB(_device, renderItems.size());

		UINT cbIdx = 0;
		for (auto ri : renderItems)
		{
			if (ri.NumFramesDirty <= 0)
				continue;
			MeshConstants objConstants;
			objConstants.mWorld = ri.World;

			ri.ObjCBIndex = cbIdx;
			frameResource->MeshCB->CopyData(ri.ObjCBIndex, objConstants);
			cbIdx++;
		}
		PIXEndEvent();
	}

	void DrawCube(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems)
	{

		// Setting Root Signature
		context.SetRootSignature(RootSig);

		// Setting global constant
		context.SetConstantBuffer(1, Resources->GetFrameResource()->GlobalCB->Resource()->GetGPUVirtualAddress());

		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, DrawHeap.GetHeapPointer());

		auto objectCB = Resources->GetFrameResource()->MeshCB->Resource();
		UINT objCBByteSize = D_RENDERER_UTILS::CalcConstantBufferByteSize(sizeof(MeshConstants));

		// For each render item
		for (auto const& ri : renderItems)
		{
			auto vbv = ri.Mesh->VertexBufferView();
			auto ibv = ri.Mesh->IndexBufferView();
			context.SetVertexBuffer(0, vbv);
			context.SetIndexBuffer(ibv);
			context.SetPrimitiveTopology(ri.PrimitiveType);

			context.SetDescriptorTable(0, DrawHeap[ri.ObjCBIndex]);
			context.DrawIndexedInstanced(ri.IndexCount, 1, ri.StartIndexLocation, ri.BaseVertexLocation, 0);
		}
	}

	// Helper method to clear the back buffers.
	void Clear(D_GRAPHICS::GraphicsContext& context, D_GRAPHICS_BUFFERS::ColorBuffer& rt, D_GRAPHICS_BUFFERS::DepthBuffer& depthStencil, RECT bounds, std::wstring const& processName)
	{

		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, processName.c_str());

		// Clear the views.
		auto const rtvDescriptor = rt.GetRTV();
		auto const dsvDescriptor = depthStencil.GetDSV();

		// Set the viewport and scissor rect.
		long width = bounds.right - bounds.left;
		long height = bounds.bottom - bounds.top;
		auto viewport = CD3DX12_VIEWPORT((float)bounds.left, (float)bounds.top, (long)width, (long)height, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissorRect = CD3DX12_RECT(bounds.left, bounds.top, (long)width, (long)height);


		context.ClearColor(rt, &scissorRect);
		context.ClearDepth(depthStencil);
		context.SetRenderTarget(rtvDescriptor, dsvDescriptor);
		context.SetViewportAndScissor(viewport, scissorRect);

		PIXEndEvent(context.GetCommandList());
	}

	void UpdateGlobalConstants(D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals)
	{
		PIXBeginEvent(PIX_COLOR_DEFAULT, "Update Globals");

		auto currGlobalCb = Resources->GetFrameResource()->GlobalCB.get();

		currGlobalCb->CopyData(0, globals);
		PIXEndEvent();
	}


#ifdef _D_EDITOR
	DescriptorHandle	GetRenderResourceHandle(UINT index)
	{
		D_ASSERT_M(index != 0, "Access to 0 index of render resouces are not allowed");
		D_ASSERT_M(index < MaxImguiElements&& index >= 1, "Index out of bound");
		return ImguiHeap[index];
	}

	void InitializeGUI()
	{
		ImguiHeap.Create(L"Imgui Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, MaxImguiElements);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Resources->GetWindow());
		ImGui_ImplDX12_Init(_device, Resources->GetBackBufferCount(), DXGI_FORMAT_B8G8R8A8_UNORM, ImguiHeap.GetHeapPointer(), ImguiHeap.GetHeapPointer()->GetCPUDescriptorHandleForHeapStart(), ImguiHeap.GetHeapPointer()->GetGPUDescriptorHandleForHeapStart());
	}
#endif

	namespace Device
	{
		void Initialize(HWND window, int width, int height)
		{
			// TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
			//   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
			//   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
			Resources = std::make_unique<DeviceResource::DeviceResources>();

			Resources->SetWindow(window, width, height);
			Resources->CreateDeviceResources();
			D_GRAPHICS::Initialize();
			Resources->CreateWindowSizeDependentResources();
			D_CAMERA_MANAGER::Initialize();
			D_CAMERA_MANAGER::SetViewportDimansion((float)width, (float)height);
		}

		void RegisterDeviceNotify(IDeviceNotify* notify)
		{
			Resources->RegisterDeviceNotify(notify);
		}

		void Shutdown()
		{
			Resources.reset();
			D_GRAPHICS::Shutdown();
			D_CAMERA_MANAGER::Shutdown();
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

		FrameResource* GetCurrentFrameResource()
		{
			return Resources->GetFrameResource();
		}

		ID3D12Device* GetDevice()
		{
			return Resources->GetD3DDevice();
		}

		FrameResource* GetFrameResourceWithIndex(int i)
		{
			return Resources->GetFrameResourceByIndex(i);
		}

		DXGI_FORMAT GetBackBufferFormat()
		{
			return Resources->GetBackBufferFormat();
		}

		DXGI_FORMAT GetDepthBufferFormat()
		{
			return Resources->GetDepthBufferFormat();
		}

		void WaitForGpu()
		{
			auto cmdManager = D_GRAPHICS::GetCommandManager();
			cmdManager->IdleGPU();
		}
	}

}