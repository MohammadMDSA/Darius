#include "pch.hpp"
#include "Renderer.hpp"
#include "Geometry/Mesh.hpp"
#include "FrameResource.hpp"
#include "GraphicsUtils/D3DUtils.hpp"
#include "GraphicsUtils/UploadBuffer.hpp"

#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

#include <Math/VectorMath.hpp>
#include <Utils/Assert.hpp>
#include <filesystem>

#include <Math/VectorMath.hpp>

using namespace Microsoft::WRL;

using namespace Darius::Math;
using namespace Darius::Renderer::DeviceResource;
using namespace Darius::Renderer::GraphicsUtils;
using namespace D_RENDERER_GEOMETRY;

namespace Darius::Renderer
{

	ID3D12Device* _device = nullptr;

	// Input layout and root signature
	//std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	//ComPtr<ID3D12RootSignature> RootSignature = nullptr;

	// Shaders
	//std::unordered_map<std::string, ComPtr<ID3DBlob>> Shaders;

	// Constant buffer view descriptor heap and object
	//std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
	//ComPtr<ID3D12DescriptorHeap> CbvHeap = nullptr;
	//ComPtr<ID3D12DescriptorHeap> ImguiHeap = nullptr;

	// PSO
	//std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> Psos;

	// Device resource
	std::unique_ptr<DeviceResource::DeviceResources> Resources;

	//UINT PassCbvOffset = 0;

	///////////////// REMOVE ASAP //////////////////
	bool _4xMsaaState = false;
	short _4xMsaaQuality = 0;

	//////////////////////////////////////////////////////
	// Functions

	void DrawCube(std::vector<RenderItem*> const& renderItems);
	void DrawImgui();

	void Clear();

	void Initialize()
	{
		D_ASSERT(_device == nullptr);
		D_ASSERT(Resources);

		_device = Resources->GetD3DDevice();

	}

	void Shutdown()
	{
		D_ASSERT(_device != nullptr);
		//DisposeUploadBuffers();

	}

	void RenderMeshes(std::vector<RenderItem*> const& renderItems)
	{
		// Prepare the command list to render a new frame.
		Resources->Prepare(D_RENDERER_DEVICE::Psos["opaque"].Get());

		// Prepare imgui
		/*ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();*/

		Clear();

		auto commandList = Resources->GetCommandList();
		PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render opaque");

		DrawCube(renderItems);

		DrawImgui();

		PIXEndEvent(commandList);

		// Show the new frame.
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
		Resources->Present();

		// If using the DirectX Tool Kit for DX12, uncomment this line:
		// m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

		PIXEndEvent();
	}

	void DrawCube(std::vector<RenderItem*> const& renderItems)
	{
		auto cmdList = Resources->GetCommandList();

		// Setting descriptor heaps
		ID3D12DescriptorHeap* descriptorHeaps[] = { D_RENDERER_DEVICE::CbvHeap.Get() };
		cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		// Setting Root Signature
		cmdList->SetGraphicsRootSignature(D_RENDERER_DEVICE::RootSignature.Get());

		// Setting global constant
		int globalCBVIndex = D_RENDERER_DEVICE::PassCbvOffset + Resources->GetCurrentFrameIndex();
		auto globalCbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D_RENDERER_DEVICE::CbvHeap->GetGPUDescriptorHandleForHeapStart());
		globalCbvHandle.Offset(globalCBVIndex, Resources->GetCbvSrvUavDescriptorSize());
		cmdList->SetGraphicsRootDescriptorTable(1, globalCbvHandle);

		auto objectCB = Resources->GetFrameResource()->MeshCB->Resource();

		// For each render item
		for (auto const& ri : renderItems)
		{
			auto vbv = ri->Mesh->VertexBufferView();
			auto ibv = ri->Mesh->IndexBufferView();
			cmdList->IASetVertexBuffers(0, 1, &vbv);
			cmdList->IASetIndexBuffer(&ibv);
			cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

			// Ofset to the CBV in the descripor heap for this object and for this frame resource
			UINT cbvIndex = Resources->GetCurrentFrameIndex() * (UINT)renderItems.size() + ri->ObjCBIndex;
			auto cbvHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(D_RENDERER_DEVICE::CbvHeap->GetGPUDescriptorHandleForHeapStart());
			cbvHandle.Offset(cbvIndex, _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

			cmdList->SetGraphicsRootDescriptorTable(0, cbvHandle);
			cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
		}
	}

	void DrawImgui()
	{
		//auto commandList = Resources->GetCommandList();

		//{
		//	static bool show_demo_window = true;
		//	static bool show_another_window = true;
		//	static float clear_color[] = { 1.f, 1.f, 1.f, 0.f };
		//	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		//	if (show_demo_window)
		//		ImGui::ShowDemoWindow(&show_demo_window);

		//	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		//	{
		//		static float f = 0.0f;
		//		static int counter = 0;

		//		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		//		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		//		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		//		ImGui::Checkbox("Another Window", &show_another_window);

		//		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		//		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		//			counter++;
		//		ImGui::SameLine();
		//		ImGui::Text("counter = %d", counter);

		//		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//		ImGui::End();
		//	}

		//	// 3. Show another simple window.
		//	if (show_another_window)
		//	{
		//		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		//		ImGui::Text("Hello from another window!");
		//		if (ImGui::Button("Close Me"))
		//			show_another_window = false;
		//		ImGui::End();
		//	}
		//}


		//ImGui::Render();

		//commandList->SetDescriptorHeaps(1, ImguiHeap.GetAddressOf());
		//ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

	}

	//void DisposeUploadBuffers()
	//{
	//	mesh->DisposeUploadBuffers();
	//}

	// Helper method to clear the back buffers.
	void Clear()
	{
		auto commandList = Resources->GetCommandList();
		PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

		// Clear the views.
		auto const rtvDescriptor = Resources->GetRenderTargetView();
		auto const dsvDescriptor = Resources->GetDepthStencilView();

		commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
		commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
		commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

		// Set the viewport and scissor rect.
		auto const viewport = Resources->GetScreenViewport();
		auto const scissorRect = Resources->GetScissorRect();
		commandList->RSSetViewports(1, &viewport);
		commandList->RSSetScissorRects(1, &scissorRect);

		PIXEndEvent(commandList);
	}

	namespace Device
	{
		ComPtr<ID3D12DescriptorHeap> CbvHeap = nullptr;
		ComPtr<ID3D12RootSignature> RootSignature = nullptr;
		std::unordered_map<std::string, ComPtr<ID3D12PipelineState>> Psos;
		UINT PassCbvOffset;
		std::unordered_map<std::string, ComPtr<ID3DBlob>> Shaders;
		std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;


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

		FrameResource* GetCurrentFrameResource()
		{
			return Resources->GetFrameResource();
		}

		ID3D12Device* GetDevice()
		{
			return _device;
		}

		ID3D12CommandAllocator* GetCommandAllocator()
		{
			return Resources->GetCommandAllocator();
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