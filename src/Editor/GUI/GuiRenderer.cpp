#include "Editor/pch.hpp"
#include "GuiRenderer.hpp"

#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>

#include <imgui.h>
#include <implot.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

using namespace D_GRAPHICS_MEMORY;

namespace Darius::Editor::Gui::Renderer
{
	DescriptorHeap										GuiTextureHeap;

	void Initialize()
	{
		GuiTextureHeap.Create(L"Imgui Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);

		auto defualtHandle = GuiTextureHeap.Alloc(1);

		ImGui::CreateContext();
		ImPlot::CreateContext();
		ImGui_ImplWin32_Init(D_GRAPHICS::GetWindow());
		ImGui_ImplDX12_Init(D_GRAPHICS_DEVICE::GetDevice(), D_GRAPHICS_DEVICE::GetBackBufferCount(), D_GRAPHICS::SwapChainGetColorFormat(), GuiTextureHeap.GetHeapPointer(), defualtHandle, defualtHandle);

		D_GRAPHICS::GetCommandManager()->IdleGPU();
	}

	void Shutdown()
	{
		GuiTextureHeap.Destroy();

	}

	DescriptorHandle AllocateUiTexture(UINT count)
	{
		return GuiTextureHeap.Alloc(count);
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
		auto viewport = CD3DX12_VIEWPORT((float)bounds.left, (float)bounds.top, (float)width, (float)height, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissorRect = CD3DX12_RECT(bounds.left, bounds.top, (long)width, (long)height);

		context.ClearColor(rt, &scissorRect);
		context.ClearDepth(depthStencil);
		context.SetRenderTarget(rtvDescriptor, dsvDescriptor);
		context.SetViewportAndScissor(viewport, scissorRect);

		PIXEndEvent(context.GetCommandList());
	}

	void Render()
	{
		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Render Gui");

		auto& viewportRt = D_GRAPHICS_DEVICE::GetRTBuffer();
		auto& depthStencil = D_GRAPHICS_DEVICE::GetDepthStencilBuffer();
		context.TransitionResource(depthStencil, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		context.TransitionResource(viewportRt, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		// Clear RT
		Clear(context, viewportRt, depthStencil, D_GRAPHICS_DEVICE::GetOutputSize(), L"Clear GUI Render Target");

		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, GuiTextureHeap.GetHeapPointer());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());
		context.Finish();
	}

}