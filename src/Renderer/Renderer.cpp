#include "Renderer.hpp"
#include "Renderer.hpp"
#include "pch.hpp"
#include "Renderer.hpp"
#include "Geometry/Mesh.hpp"
#include "FrameResource.hpp"
#include "GraphicsUtils/D3DUtils.hpp"
#include "GraphicsUtils/UploadBuffer.hpp"
#include "GraphicsUtils/Memory/DescriptorHeap.hpp"
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

namespace Darius::Renderer
{

	ID3D12Device* _device = nullptr;

	// Input layout and root signature
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	ComPtr<ID3D12RootSignature> RootSignature = nullptr;

	// Shaders
	std::unordered_map<std::string, ComPtr<ID3DBlob>> Shaders;

#ifdef _D_EDITOR
	std::function<void(void)>		GuiDrawer = nullptr;
	DescriptorHeap					ImguiHeap;
#endif

	// PSO
	std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> Psos;

	// Device resource
	std::unique_ptr<DeviceResource::DeviceResources> Resources;

	UINT PassCbvOffset = 0;

	///////////////// Heaps ///////////////////////
	DescriptorHeap					SceneTexturesHeap;


	///////////////// REMOVE ASAP //////////////////
	bool _4xMsaaState = false;
	short _4xMsaaQuality = 0;

	//////////////////////////////////////////////////////
	// Functions
#ifdef _D_EDITOR
	void InitializeGUI();
#endif

	void DrawCube(std::vector<RenderItem*> const& renderItems);

	void UpdateGlobalConstants();

	void Clear();

	void Initialize()
	{
		D_ASSERT(_device == nullptr);
		D_ASSERT(Resources);

		_device = Resources->GetD3DDevice();

#ifdef _D_EDITOR
		InitializeGUI();
#endif // _D_EDITOR

		SceneTexturesHeap.Create(L"Scene Textures", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);
	}

	void Shutdown()
	{
		D_ASSERT(_device != nullptr);
		//DisposeUploadBuffers();

	}

#ifdef _D_EDITOR
	void RegisterGuiDrawer(std::function<void(void)> drawer)
	{
		GuiDrawer = drawer;
	}
#endif

	void RenderMeshes(std::vector<RenderItem*> const& renderItems)
	{
		UpdateGlobalConstants();

		// Prepare the command list to render a new frame.
		Resources->Prepare(Psos["opaque"].Get());

		Clear();

		auto commandList = Resources->GetCommandList();
		PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render opaque");

		DrawCube(renderItems);
		PIXEndEvent(commandList);

#ifdef _D_EDITOR
		PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render gui");
		if (GuiDrawer)
		{

			// Prepare imgui
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			GuiDrawer();

			ID3D12DescriptorHeap* desHeaps[] = { ImguiHeap.GetHeapPointer() };

			commandList->SetDescriptorHeaps(1, desHeaps);
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
		}
		PIXEndEvent(commandList);
#endif

		// Show the new frame.
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
		Resources->Present();

		// If using the DirectX Tool Kit for DX12, uncomment this line:
		// m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

		PIXEndEvent();
	}

	void UpdateMeshCBs(std::vector<RenderItem*> const& renderItems)
	{
		PIXBeginEvent(PIX_COLOR_INDEX(10), "Update Mesh CBs");
		auto frameResource = Resources->GetFrameResource();
		frameResource->ReinitializeMeshCB(_device, renderItems.size());

		for (auto& ri : renderItems)
		{
			if (ri->NumFramesDirty <= 0)
				continue;
			MeshConstants objConstants;
			objConstants.mWorld = ri->World;

			frameResource->MeshCB->CopyData(ri->ObjCBIndex, objConstants);

			// Next FrameResource needs to be updated too.
			//ri->NumFramesDirty--;
		}
		PIXEndEvent();
	}

	void DrawCube(std::vector<RenderItem*> const& renderItems)
	{
		auto cmdList = Resources->GetCommandList();

		// Setting Root Signature
		cmdList->SetGraphicsRootSignature(RootSignature.Get());

		// Setting global constant
		int globalCBVIndex = PassCbvOffset + Resources->GetCurrentFrameResourceIndex();

		cmdList->SetGraphicsRootConstantBufferView(1, Resources->GetFrameResource()->GlobalCB->Resource()->GetGPUVirtualAddress());

		auto objectCB = Resources->GetFrameResource()->MeshCB->Resource();
		UINT objCBByteSize = D_RENDERER_UTILS::CalcConstantBufferByteSize(sizeof(MeshConstants));

		// For each render item
		for (auto const& ri : renderItems)
		{
			auto vbv = ri->Mesh->VertexBufferView();
			auto ibv = ri->Mesh->IndexBufferView();
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetIndexBuffer(&ibv);
			cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

			cmdList->SetGraphicsRootConstantBufferView(0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);
			cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}
	}

	// Helper method to clear the back buffers.
	void Clear()
	{
		auto commandList = Resources->GetCommandList();
		PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

		// Clear the views.
		auto const rtvDescriptor = Resources->GetRenderTargetView();
		auto const dsvDescriptor = Resources->GetDepthStencilView();

		commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
		commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		commandList->OMSetRenderTargets(1, &rtvDescriptor, true, &dsvDescriptor);

		// Set the viewport and scissor rect.
		auto const viewport = Resources->GetScreenViewport();
		auto const scissorRect = Resources->GetScissorRect();
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		PIXEndEvent(commandList);
	}

	void UpdateGlobalConstants()
	{
		PIXBeginEvent(PIX_COLOR_DEFAULT, "Update Globals");
		D_RENDERER_FRAME_RESOUCE::GlobalConstants globals;

		auto camera = D_CAMERA_MANAGER::GetActiveCamera();

		auto view = camera->GetViewMatrix();
		auto proj = camera->GetProjMatrix();

		auto viewProj = camera->GetViewProjMatrix();
		auto invView = Matrix4::Inverse(view);
		auto invProj = Matrix4::Inverse(proj);
		auto invViewProj = Matrix4::Inverse(viewProj);

		float width, height;
		D_CAMERA_MANAGER::GetViewportDimansion(width, height);

		auto time = *D_TIME::GetStepTimer();

		globals.View = view;
		globals.InvView = invView;
		globals.Proj = proj;
		globals.InvProj = invProj;
		globals.ViewProj = viewProj;
		globals.InvViewProj = invViewProj;
		globals.CameraPos = camera->GetPosition();
		globals.RenderTargetSize = XMFLOAT2(width, height);
		globals.InvRenderTargetSize = XMFLOAT2(1.f / width, 1.f / height);
		globals.NearZ = camera->GetNearClip();
		globals.FarZ = camera->GetFarClip();
		globals.TotalTime = (float)time.GetTotalSeconds();
		globals.DeltaTime = (float)time.GetElapsedSeconds();

		auto currGlobalCb = Resources->GetFrameResource()->GlobalCB.get();

		currGlobalCb->CopyData(0, globals);
		PIXEndEvent();
	}

#ifdef _D_EDITOR
	void InitializeGUI()
	{
		ImguiHeap.Create(L"Imgui Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Resources->GetWindow());
		ImGui_ImplDX12_Init(_device, Resources->GetBackBufferCount(), DXGI_FORMAT_B8G8R8A8_UNORM, ImguiHeap.GetHeapPointer(), ImguiHeap.GetHeapPointer()->GetCPUDescriptorHandleForHeapStart(), ImguiHeap.GetHeapPointer()->GetGPUDescriptorHandleForHeapStart());
	}
#endif

	//size_t GetSceneTextureHandle()
	//{
	//	//auto handle = Resources->GetRenderTargetShaderResourceHandle();
	//	return 0;
	//}

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
			Resources->CreateWindowSizeDependentResources();
		}

		void RegisterDeviceNotify(IDeviceNotify* notify)
		{
			Resources->RegisterDeviceNotify(notify);
		}

		void Shutdown()
		{
			if (Resources)
				Resources->WaitForGpu();
			Resources.release();
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

		void SyncFrame()
		{
			PIXBeginEvent(PIX_COLOR(255, 0, 255), "Syncing");
			Resources->SyncFrameStartGPU();
			PIXEndEvent();
		}

		FrameResource* GetCurrentFrameResource()
		{
			return Resources->GetFrameResource();
		}

		ID3D12Device* GetDevice()
		{
			return Resources->GetD3DDevice();
		}

		ID3D12CommandAllocator* GetCommandAllocator()
		{
			return Resources->GetDirectCommandAllocator();
		}

		ID3D12GraphicsCommandList* GetCommandList()
		{
			return Resources->GetCommandList();
		}

		ID3D12CommandQueue* GetCommandQueue()
		{
			return Resources->GetCommandQueue();
		}

		FrameResource* GetFrameResourceWithIndex(int i)
		{
			return Resources->GetFrameResourceWithIndex(i);
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
			Resources->WaitForGpu();
		}
	}

}