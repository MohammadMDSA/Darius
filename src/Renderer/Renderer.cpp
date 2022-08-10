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

#ifdef _D_EDITOR
	std::function<void(void)>							GuiDrawer = nullptr;
	DescriptorHeap										ImguiHeap;
	UINT												MaxImguiElements = 2;
#endif

	// Input layout and root signature
	std::vector<D3D12_INPUT_ELEMENT_DESC>				InputLayout;
	D_GRAPHICS_UTILS::RootSignature						RootSign;
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
	void BuildPSO();
	void BuildRootSignature();

	void Clear(D_GRAPHICS::GraphicsContext& context, D_GRAPHICS_BUFFERS::ColorBuffer& rt, D_GRAPHICS_BUFFERS::DepthBuffer& depthStencil, RECT bounds, std::wstring const& processName = L"Clear");

	void Initialize()
	{
		D_ASSERT(_device == nullptr);
		D_ASSERT(Resources);

		_device = Resources->GetD3DDevice();

		BuildRootSignature();
		BuildPSO();

		// Create Constant Buffer Views
		UINT objSize = D_RENDERER_UTILS::CalcConstantBufferByteSize(sizeof(MeshConstants));

#ifdef _D_EDITOR
		InitializeGUI();
#endif // _D_EDITOR

	}

	void Shutdown()
	{
		D_ASSERT(_device != nullptr);
		ImguiHeap.Destroy();
		RootSign.DestroyAll();
		for (auto& kv : Psos)
			kv.second.DestroyAll();
	}

#ifdef _D_EDITOR
	void RegisterGuiDrawer(std::function<void(void)> drawer)
	{
		GuiDrawer = drawer;
	}
#endif

	void RenderMeshes(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems, D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals)
	{

		// Setting Root Signature
		context.SetRootSignature(D_RENDERER::RootSign);

		context.SetDynamicConstantBufferView(kCommonCBV, sizeof(GlobalConstants), &globals);

		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, L"Render opaque");

		// Draw each render item
		for (auto const& ri : renderItems)
		{
			auto vbv = ri.Mesh->VertexBufferView();
			auto ibv = ri.Mesh->IndexBufferView();
			context.SetVertexBuffer(0, vbv);
			context.SetIndexBuffer(ibv);
			context.SetPrimitiveTopology(ri.PrimitiveType);

			context.SetConstantBuffer(kMeshConstants, ri.CBVGpu);
			context.DrawIndexedInstanced(ri.IndexCount, 1, ri.StartIndexLocation, ri.BaseVertexLocation, 0);
		}

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

	void BuildPSO()
	{
		InputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};

		// For Opaque objects
		GraphicsPSO pso(L"Opaque");

		pso.SetInputLayout((UINT)InputLayout.size(), InputLayout.data());

		pso.SetVertexShader(reinterpret_cast<BYTE*>(Shaders["standardVS"]->GetBufferPointer()),
			Shaders["standardVS"]->GetBufferSize());
		pso.SetPixelShader(reinterpret_cast<BYTE*>(Shaders["opaquePS"]->GetBufferPointer()),
			Shaders["opaquePS"]->GetBufferSize());
		auto rasterState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rasterState.FillMode = D3D12_FILL_MODE_SOLID;
		rasterState.CullMode = D3D12_CULL_MODE_FRONT;
		pso.SetRootSignature(RootSign);
		pso.SetRasterizerState(rasterState);
		pso.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		pso.SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		pso.SetSampleMask(UINT_MAX);
		pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		pso.SetRenderTargetFormat(D_RENDERER_DEVICE::GetBackBufferFormat(), D_RENDERER_DEVICE::GetDepthBufferFormat());
		pso.Finalize();
		Psos["opaque"] = pso;


		// For opaque wireframe objecs
		GraphicsPSO wirePso(pso);
		auto wireRasterState = rasterState;
		wireRasterState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		wirePso.SetRasterizerState(wireRasterState);
		wirePso.Finalize();
		Psos["opaque_wireframe"] = wirePso;
	}

	void BuildRootSignature()
	{
		// Root parameter can be a table, root descriptor or root constants.
		D_GRAPHICS_UTILS::RootParameter slotRootParameter[2];

		// A root signature is an array of root parameters.
		RootSign.Reset(kNumRootBindings, 0);

		// Create root CBVs.
		RootSign[kMeshConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
		RootSign[kMaterialConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
		RootSign[kCommonCBV].InitAsConstantBuffer(1);

		RootSign.Finalize(L"Main Root Sig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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

		UINT GetCurrentResourceIndex()
		{
			return Resources->GetCurrentFrameResourceIndex();
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