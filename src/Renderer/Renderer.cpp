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

	// Shaders
	std::unordered_map<std::string, ComPtr<ID3DBlob>>	Shaders;

#ifdef _D_EDITOR
	std::function<void(void)>							GuiDrawer = nullptr;
	DescriptorHeap										ImguiHeap;
#endif

	// PSO
	std::unordered_map<std::string, D_GRAPHICS_UTILS::GraphicsPSO> Psos;

	// Device resource
	std::unique_ptr<DeviceResource::DeviceResources>	Resources;

	UINT PassCbvOffset = 0;

	///////////////// Heaps ///////////////////////
	DescriptorHeap										SceneTexturesHeap;

	///////////////// REMOVE ASAP //////////////////
	bool _4xMsaaState = false;
	short _4xMsaaQuality = 0;

	///////////////// Render Textures //////////////
	float												SceneWidth;
	float												SceneHeight;
	D_GRAPHICS_BUFFERS::ColorBuffer						SceneTexture;
	D_GRAPHICS_BUFFERS::DepthBuffer						SceneDepth;

	//////////////////////////////////////////////////////
	// Functions

#ifdef _D_EDITOR
	void InitializeGUI();
#endif

	void DrawCube(D_GRAPHICS::GraphicsContext& context, std::vector<RenderItem*> const& renderItems);

	void UpdateGlobalConstants();

	void Clear(D_GRAPHICS::GraphicsContext& context, D_GRAPHICS_BUFFERS::ColorBuffer& rt, D_GRAPHICS_BUFFERS::DepthBuffer& depthStencil, RECT bounds, std::wstring const& processName = L"Clear");

	void Initialize()
	{
		D_ASSERT(_device == nullptr);
		D_ASSERT(Resources);

		_device = Resources->GetD3DDevice();

#ifdef _D_EDITOR
		InitializeGUI();
#endif // _D_EDITOR

		SceneTexturesHeap.Create(L"Scene Textures", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);

		// Create scene rt and ds
		D_CAMERA_MANAGER::GetViewportDimansion(SceneWidth, SceneHeight);
		SceneTexture.Create(L"Scene Texture", (UINT)SceneWidth, (UINT)SceneHeight, 1, Resources->GetBackBufferFormat());
		SceneDepth.Create(L"Scene DepthStencil", (UINT)SceneWidth, (UINT)SceneHeight, Resources->GetDepthBufferFormat());
	}

	void Shutdown()
	{
		D_ASSERT(_device != nullptr);
		SceneDepth.Destroy();
		SceneTexture.Destroy();
		SceneTexturesHeap.Destroy();

	}

#ifdef _D_EDITOR
	void RegisterGuiDrawer(std::function<void(void)> drawer)
	{
		GuiDrawer = drawer;
	}
#endif

	void SetRendererDimansions(float width, float height)
	{
		width = XMMax(width, 1.f);
		height = XMMax(height, 1.f);
		if (width == SceneWidth && height == SceneHeight)
			return;
		SceneWidth = width;
		SceneHeight = height;
		SceneTexture.Create(L"Scene Texture", (UINT)SceneWidth, (UINT)SceneHeight, 1, Resources->GetBackBufferFormat());
		SceneDepth.Create(L"Scene DepthStencil", (UINT)SceneWidth, (UINT)SceneHeight, Resources->GetDepthBufferFormat());
	}

	void RenderMeshes(D_GRAPHICS::GraphicsContext& context, std::vector<RenderItem*> const& renderItems)
	{
		UpdateGlobalConstants();

		context.SetPipelineState(Psos["opaque"]);
		// Prepare the command list to render a new frame.
		context.TransitionResource(SceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		RECT bounds = { 0l, 0l, (long)SceneWidth, (long)SceneHeight };
		Clear(context, SceneTexture, SceneDepth, bounds, L"Clear Scene");

		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, L"Render opaque");

		DrawCube(context, renderItems);

#ifdef _D_EDITOR
		context.TransitionResource(SceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		_device->CopyDescriptorsSimple(1, ImguiHeap[1], SceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
#else
		context.TransitionResource(SceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
#endif
		PIXEndEvent(context.GetCommandList());

#ifdef _D_EDITOR
		//PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render gui");
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

			context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ImguiHeap.GetHeapPointer());
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());
		}
		//PIXEndEvent(commandList);
#endif
		// Show the new frame.
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
		Resources->Present(context);

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

	void DrawCube(D_GRAPHICS::GraphicsContext& context, std::vector<RenderItem*> const& renderItems)
	{

		// Setting Root Signature
		context.SetRootSignature(RootSig);

		// Setting global constant
		context.SetConstantBuffer(1, Resources->GetFrameResource()->GlobalCB->Resource()->GetGPUVirtualAddress());

		auto objectCB = Resources->GetFrameResource()->MeshCB->Resource();
		UINT objCBByteSize = D_RENDERER_UTILS::CalcConstantBufferByteSize(sizeof(MeshConstants));

		// For each render item
		for (auto const& ri : renderItems)
		{
			auto vbv = ri->Mesh->VertexBufferView();
			auto ibv = ri->Mesh->IndexBufferView();
			context.SetVertexBuffer(0, vbv);
			context.SetIndexBuffer(ibv);
			context.SetPrimitiveTopology(ri->PrimitiveType);

			context.SetConstantBuffer(0, objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize);
			context.DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
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
		ImguiHeap.Create(L"Imgui Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 2);

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Resources->GetWindow());
		ImGui_ImplDX12_Init(_device, Resources->GetBackBufferCount(), DXGI_FORMAT_B8G8R8A8_UNORM, ImguiHeap.GetHeapPointer(), ImguiHeap.GetHeapPointer()->GetCPUDescriptorHandleForHeapStart(), ImguiHeap.GetHeapPointer()->GetGPUDescriptorHandleForHeapStart());
	}
#endif

	D3D12_GPU_DESCRIPTOR_HANDLE GetSceneTextureHandle()
	{
		return ImguiHeap[1];
	}

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
			auto cmdManager = D_GRAPHICS::GetCommandManager();
			cmdManager->IdleGPU();
		}
	}

}