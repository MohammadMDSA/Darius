#include "Renderer.hpp"
#include "pch.hpp"

#include "Renderer.hpp"
#include "Geometry/Mesh.hpp"
#include "FrameResource.hpp"
#include "GraphicsCore.hpp"
#include "GraphicsUtils/D3DUtils.hpp"
#include "GraphicsUtils/RootSignature.hpp"
#include "GraphicsUtils/Memory/DescriptorHeap.hpp"
#include "GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "GraphicsUtils/VertexTypes.hpp"
#include "Camera/CameraManager.hpp"

#include <imgui.h>
#include <implot/implot.h>
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
	DescriptorHeap										ImguiHeap;
	UINT												MaxImguiElements = 2;
#endif

	// Input layout and root signature
	std::array<D_GRAPHICS_UTILS::RootSignature, (size_t)RootSignatureTypes::_numRootSig> RootSigns;
	std::array<D_GRAPHICS_UTILS::GraphicsPSO, (size_t)PipelineStateTypes::_numPso> Psos;

	// Device resource
	std::unique_ptr<DeviceResource::DeviceResources>	Resources;

	DescriptorHeap										TextureHeap;
	DescriptorHeap										SamplerHeap;

	DescriptorHandle									CommonTexture;
	DescriptorHandle									CommonTextureSamplers;

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

		TextureHeap.Create(L"Scene Texture Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096);
		SamplerHeap.Create(L"Scene Sampler Descriptors", D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 2048);

		CommonTexture = TextureHeap.Alloc(8);
		CommonTextureSamplers = SamplerHeap.Alloc(kNumTextures);

#ifdef _D_EDITOR
		InitializeGUI();
#endif // _D_EDITOR

	}

	void Shutdown()
	{
		D_ASSERT(_device != nullptr);

		ImguiHeap.Destroy();
		TextureHeap.Destroy();
		SamplerHeap.Destroy();
		D_GRAPHICS_UTILS::RootSignature::DestroyAll();
		D_GRAPHICS_UTILS::GraphicsPSO::DestroyAll();
	}

	D_GRAPHICS_UTILS::GraphicsPSO& GetPSO(PipelineStateTypes type)
	{
		return Psos[(size_t)type];
	}

	D_GRAPHICS_UTILS::RootSignature& GetRootSignature(RootSignatureTypes type)
	{
		return RootSigns[(size_t)type];
	}

	void Present(D_GRAPHICS::GraphicsContext& context)
	{
		// Show the new frame.
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
		Resources->Present(context);

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
		// For Opaque objects
		Psos[(size_t)PipelineStateTypes::OpaquePso] = GraphicsPSO(L"Opaque PSO");
		auto& pso = Psos[(size_t)PipelineStateTypes::OpaquePso];

		auto il = D_RENDERER_VERTEX::VertexPositionNormalTexture::InputLayout;
		pso.SetInputLayout(il.NumElements, il.pInputElementDescs);

		pso.SetVertexShader(reinterpret_cast<BYTE*>(Shaders["standardVS"]->GetBufferPointer()),
			Shaders["standardVS"]->GetBufferSize());
		pso.SetPixelShader(reinterpret_cast<BYTE*>(Shaders["opaquePS"]->GetBufferPointer()),
			Shaders["opaquePS"]->GetBufferSize());
		pso.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		pso.SetRasterizerState(D_GRAPHICS::RasterizerDefault);
		pso.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		pso.SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		pso.SetSampleMask(UINT_MAX);
		pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		pso.SetRenderTargetFormat(D_RENDERER_DEVICE::GetBackBufferFormat(), D_RENDERER_DEVICE::GetDepthBufferFormat());
		pso.Finalize();

		// Transparent
		Psos[(size_t)PipelineStateTypes::TransparentPso] = GraphicsPSO(L"Transparent PSO");
		auto& transPso = Psos[(size_t)PipelineStateTypes::TransparentPso];
		transPso = pso;
		transPso.SetBlendState(D_GRAPHICS::BlendTraditional);
		transPso.Finalize();

		// For opaque wireframe objecs
		Psos[(size_t)PipelineStateTypes::WireframePso] = pso;
		auto& wirePso = Psos[(size_t)PipelineStateTypes::WireframePso];
		auto wireRasterState = D_GRAPHICS::RasterizerDefaultWireframe;
		wireRasterState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		wirePso.SetRasterizerState(wireRasterState);
		wirePso.Finalize(L"Wireframe");

		// For colored only objects
		Psos[(size_t)PipelineStateTypes::ColorPso] = pso;
		auto& colorPso = Psos[(size_t)PipelineStateTypes::ColorPso];
		il = D_RENDERER_VERTEX::VertexPosition::InputLayout;
		colorPso.SetInputLayout(il.NumElements, il.pInputElementDescs);
		colorPso.SetVertexShader(reinterpret_cast<BYTE*>(Shaders["colorVS"]->GetBufferPointer()), Shaders["colorVS"]->GetBufferSize());
		colorPso.SetPixelShader(reinterpret_cast<BYTE*>(Shaders["colorPS"]->GetBufferPointer()), Shaders["colorPS"]->GetBufferSize());
		colorPso.SetRootSignature(RootSigns[(size_t)RootSignatureTypes::DefaultRootSig]);
		colorPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
		colorPso.Finalize(L"Color");

		{
			// For colored wireframe objecs
			Psos[(size_t)PipelineStateTypes::ColorWireframePso] = Psos[(size_t)PipelineStateTypes::ColorPso];
			auto& wireColorPso = Psos[(size_t)PipelineStateTypes::ColorWireframePso];
			wireColorPso.SetRasterizerState(D_GRAPHICS::RasterizerDefaultWireframe);
			wireColorPso.Finalize(L"Color Wireframe");
		}

		{
			// For colored wireframe objecs
			Psos[(size_t)PipelineStateTypes::ColorWireframeTwoSidedPso] = Psos[(size_t)PipelineStateTypes::ColorWireframePso];
			auto& wireColorPso = Psos[(size_t)PipelineStateTypes::ColorWireframeTwoSidedPso];
			wireColorPso.SetRasterizerState(D_GRAPHICS::RasterizerTwoSidedMsaaWireframe);
			wireColorPso.Finalize(L"Color Wireframe Two Sided");
		}
	}

	void BuildRootSignature()
	{
		// Default root signature
		auto& def = RootSigns[(size_t)RootSignatureTypes::DefaultRootSig];
		def.Reset(kNumRootBindings, 3);

		// Create samplers
		SamplerDesc defaultSamplerDesc;
		defaultSamplerDesc.MaxAnisotropy = 8;
		SamplerDesc cubeMapSamplerDesc = defaultSamplerDesc;
		def.InitStaticSampler(10, defaultSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(11, SamplerShadowDesc, D3D12_SHADER_VISIBILITY_PIXEL);
		def.InitStaticSampler(12, cubeMapSamplerDesc, D3D12_SHADER_VISIBILITY_PIXEL);

		// Create root CBVs.
		def[kMeshConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_VERTEX);
		def[kMaterialConstants].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kMaterialSRVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 10, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kMaterialSamplers].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, 0, 10, D3D12_SHADER_VISIBILITY_PIXEL);
		def[kCommonCBV].InitAsConstantBuffer(1);
		def[kCommonSRVs].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 10, 8, D3D12_SHADER_VISIBILITY_PIXEL);
		def.Finalize(L"Main Root Sig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	}

	DescriptorHandle AllocateTextureDescriptor(UINT count)
	{
		return TextureHeap.Alloc(count);
	}

	void MeshSorter::AddMesh(RenderItem const& renderItem, float distance)
	{
		SortKey key;
		key.value = m_SortObjects.size();

		bool alphaBlend = renderItem.PsoFlags & RenderItem::AlphaBlend;
		bool alphaTest = renderItem.PsoFlags & RenderItem::AlphaTest;

		union float_or_int { float f; uint32_t u; } dist;
		dist.f = Max(distance, 0.0f);

		if (renderItem.PsoFlags & RenderItem::AlphaBlend)
		{
			key.passID = kTransparent;
			key.psoIdx = renderItem.PsoFlags & RenderItem::Wireframe ? WireframePso : TransparentPso;
			key.key = ~dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kTransparent]++;
		}
		/*else if (alphaTest)
		{
			key.passID = kZPass;
			key.psoIdx = depthPSO;
			key.key = dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kZPass]++;

			key.passID = kOpaque;
			key.psoIdx = renderItem.pso + 1;
			key.key = dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kOpaque]++;
		}*/
		else
		{
			key.passID = kOpaque;
			key.psoIdx = renderItem.PsoType;
			key.key = dist.u;
			m_SortKeys.push_back(key.value);
			m_PassCounts[kOpaque]++;
		}

		SortObject object = { renderItem };
		m_SortObjects.push_back(object);
	}

	void MeshSorter::Sort()
	{
		struct { bool operator()(uint64_t a, uint64_t b) const { return a < b; } } Cmp;
		std::sort(m_SortKeys.begin(), m_SortKeys.end(), Cmp);
	}

	void MeshSorter::RenderMeshes(
		DrawPass pass,
		D_GRAPHICS::GraphicsContext& context,
		GlobalConstants& globals)
	{
		D_ASSERT(m_DSV != nullptr);

		context.PIXBeginEvent(L"Render Meshes");

		context.SetRootSignature(RootSigns[DefaultRootSig]);

		context.SetDynamicConstantBufferView(kCommonCBV, sizeof(GlobalConstants), &globals);

		// Setting up common texture (light for now)
		UINT destCount = 2;
		UINT sourceCounts[] = { 1, 1 };
		D3D12_CPU_DESCRIPTOR_HANDLE lightHandles[] =
		{
			D_LIGHT::GetLightMaskHandle(),
			D_LIGHT::GetLightDataHandle()
		};
		_device->CopyDescriptors(1, &CommonTexture, &destCount, destCount, lightHandles, sourceCounts, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, TextureHeap.GetHeapPointer());
		context.SetDescriptorTable(kCommonSRVs, CommonTexture);

		// Setup samplers
		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SamplerHeap.GetHeapPointer());
		context.SetDescriptorTable(kMaterialSamplers, CommonTextureSamplers);

		if (m_BatchType == kShadows)
		{
			context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
			context.ClearDepth(*m_DSV);
			context.SetDepthStencilTarget(m_DSV->GetDSV());

			if (m_Viewport.Width == 0)
			{
				m_Viewport.TopLeftX = 0.0f;
				m_Viewport.TopLeftY = 0.0f;
				m_Viewport.Width = (float)m_DSV->GetWidth();
				m_Viewport.Height = (float)m_DSV->GetHeight();
				m_Viewport.MaxDepth = 1.0f;
				m_Viewport.MinDepth = 0.0f;

				m_Scissor.left = 1;
				m_Scissor.right = m_DSV->GetWidth() - 2;
				m_Scissor.top = 1;
				m_Scissor.bottom = m_DSV->GetHeight() - 2;
			}
		}
		else
		{
			for (uint32_t i = 0; i < m_NumRTVs; ++i)
			{
				D_ASSERT(m_RTV[i] != nullptr);
				D_ASSERT(m_DSV->GetWidth() == m_RTV[i]->GetWidth());
				D_ASSERT(m_DSV->GetHeight() == m_RTV[i]->GetHeight());
			}

			if (m_Viewport.Width == 0)
			{
				m_Viewport.TopLeftX = 0.0f;
				m_Viewport.TopLeftY = 0.0f;
				m_Viewport.Width = (float)m_DSV->GetWidth();
				m_Viewport.Height = (float)m_DSV->GetHeight();
				m_Viewport.MaxDepth = 1.0f;
				m_Viewport.MinDepth = 0.0f;

				m_Scissor.left = 0;
				m_Scissor.right = m_DSV->GetWidth();
				m_Scissor.top = 0;
				m_Scissor.bottom = m_DSV->GetWidth();
			}
		}

		for (; m_CurrentPass <= pass; m_CurrentPass = (DrawPass)(m_CurrentPass + 1))
		{
			const uint32_t passCount = m_PassCounts[m_CurrentPass];
			if (passCount == 0)
				continue;

			if (m_BatchType == kDefault)
			{
				switch (m_CurrentPass)
				{
				case kZPass:
					/*context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					context.SetDepthStencilTarget(m_DSV->GetDSV());
					break;*/
					continue;
				case kOpaque:
					context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_WRITE);
					context.TransitionResource(*m_RTV[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
					context.SetRenderTarget(m_RTV[0]->GetRTV(), m_DSV->GetDSV());
					break;
				case kTransparent:
					context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_DEPTH_READ);
					context.TransitionResource(*m_RTV[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
					context.SetRenderTarget(m_RTV[0]->GetRTV(), m_DSV->GetDSV_DepthReadOnly());
					break;
				}
			}

			context.SetViewportAndScissor(m_Viewport, m_Scissor);
			context.FlushResourceBarriers();

			const uint32_t lastDraw = m_CurrentDraw + passCount;

			while (m_CurrentDraw < lastDraw)
			{
				SortKey key;
				key.value = m_SortKeys[m_CurrentDraw];
				const SortObject& object = m_SortObjects[key.objectIdx];
				RenderItem const& ri = object.renderItem;

				context.SetPrimitiveTopology(ri.PrimitiveType);

				context.SetConstantBuffer(kMeshConstants, ri.MeshCBV);

				if (ri.PsoFlags & RenderItem::ColorOnly)
				{
					D_RENDERER_FRAME_RESOUCE::ColorConstants color = { ri.Color };
					context.SetDynamicConstantBufferView(kMaterialConstants, sizeof(ColorConstants), &color);
				}
				else
				{
					context.SetConstantBuffer(kMaterialConstants, ri.Material.MaterialCBV);
					context.SetDescriptorTable(kMaterialSRVs, ri.Material.MaterialSRV);
				}

				context.SetPipelineState(Psos[key.psoIdx]);

				context.SetVertexBuffer(0, ri.Mesh->VertexBufferView());
				context.SetIndexBuffer(ri.Mesh->IndexBufferView());

				// TODO: Render submeshes one by one
				//for (uint32_t i = 0; i < ri.Mesh->mDraw; ++i)
					//context.DrawIndexed(mesh.draw[i].primCount, mesh.draw[i].startIndex, mesh.draw[i].baseVertex);
				context.DrawIndexedInstanced(ri.IndexCount, 1, ri.StartIndexLocation, ri.BaseVertexLocation, 0);

				++m_CurrentDraw;
			}
		}

		if (m_BatchType == kShadows)
		{
			context.TransitionResource(*m_DSV, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}

		context.PIXEndEvent();
	}

#ifdef _D_EDITOR
	DescriptorHandle	GetUiTextureHandle(UINT index)
	{
		D_ASSERT_M(index != 0, "Access to 0 index of render resouces are not allowed");
		D_ASSERT_M(index < MaxImguiElements&& index >= 1, "Index out of bound");
		return ImguiHeap[index];
	}

	void RenderGui(D_GRAPHICS::GraphicsContext& context)
	{
		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, L"Render gui");
		auto& viewportRt = Resources->GetRTBuffer();
		context.TransitionResource(viewportRt, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		// Clear RT
		Clear(context, Resources->GetRTBuffer(), Resources->GetDepthStencilBuffer(), Resources->GetOutputSize());

		context.SetDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, ImguiHeap.GetHeapPointer());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), context.GetCommandList());

		PIXEndEvent(context.GetCommandList());
	}

	void InitializeGUI()
	{
		ImguiHeap.Create(L"Imgui Heap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, MaxImguiElements);

		ImGui::CreateContext();
		ImPlot::CreateContext();
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