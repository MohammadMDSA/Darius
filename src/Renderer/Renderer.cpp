#include "pch.hpp"
#include "Renderer.hpp"
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

namespace Darius::Renderer
{

	struct ObjectConstants
	{
		XMFLOAT4X4 WorldViewProj;
	};

	ID3D12Device* _device = nullptr;

	// Vertex and index buffers
	ComPtr<ID3D12Resource> VertexBuffer = nullptr;
	ComPtr<ID3D12Resource> IndexBuffer = nullptr;

	// Upload buffers
	ComPtr<ID3D12Resource> VertexUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> IndexUploadBuffer = nullptr;

	// Data about the buffers.
	UINT VertexByteStride = 0;
	UINT VertexBufferByteSize = 0;
	DXGI_FORMAT IndexFormat = DXGI_FORMAT_R16_UINT;
	UINT IndexBufferByteSize = 0;

	// Input layout and root signature
	std::vector<D3D12_INPUT_ELEMENT_DESC> InputLayout;
	ComPtr<ID3D12RootSignature> RootSignature = nullptr;

	// Shaders
	ComPtr<ID3DBlob> VsByteCode = nullptr;
	ComPtr<ID3DBlob> PsByteCode = nullptr;

	// Constant buffer view descriptor heap and object
	std::unique_ptr<UploadBuffer<ObjectConstants>> ObjectCB = nullptr;
	ComPtr<ID3D12DescriptorHeap> CbvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> ImguiHeap = nullptr;

	// PSO
	ComPtr<ID3D12PipelineState> Pso = nullptr;

	// Device resource
	std::unique_ptr<DeviceResource::DeviceResources> Resources;

	///////////////// REMOVE ASAP //////////////////
	bool _4xMsaaState = false;
	short _4xMsaaQuality = 0;

	//////////////////////////////////////////////////////
	// Functions
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildGeometery();
	void BuildPSO();
	void BuildImgui();
	void DisposeUploadBuffers();

	void DrawCube();
	void DrawImgui();

	void Clear();

	void Initialize()
	{
		D_ASSERT(_device == nullptr);
		D_ASSERT(Resources);

		_device = Resources->GetD3DDevice();
		auto cmdList = Resources->GetCommandList();

		D_HR_CHECK(Resources->GetCommandAllocator()->Reset());
		D_HR_CHECK(cmdList->Reset(Resources->GetCommandAllocator(), nullptr));

		BuildDescriptorHeaps();
		BuildConstantBuffers();
		BuildRootSignature();
		BuildShadersAndInputLayout();
		BuildGeometery();
		BuildPSO();
		BuildImgui();

		// Execute the initialization commands.
		D_HR_CHECK(cmdList->Close());
		ID3D12CommandList* commandLists[] = { cmdList };
		Resources->GetCommandQueue()->ExecuteCommandLists(_countof(commandLists), commandLists);

		// Wait until gpu executes initial commands
		Resources->WaitForGpu();
	}

	void Shutdown()
	{
		D_ASSERT(_device != nullptr);

	}

	void RenderMeshes()
	{

		// Prepare the command list to render a new frame.
		Resources->Prepare(Pso.Get());

		// Prepare imgui
		/*ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();*/

		Clear();

		auto commandList = Resources->GetCommandList();
		PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

		DrawCube();

		//DrawImgui();

		PIXEndEvent(commandList);

		// Show the new frame.
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Present");
		Resources->Present();

		// If using the DirectX Tool Kit for DX12, uncomment this line:
		// m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());

		PIXEndEvent();
	}

	void DrawCube()
	{
		auto cmdList = Resources->GetCommandList();

		// Setting descriptor heaps
		ID3D12DescriptorHeap* descriptorHeaps[] = { CbvHeap.Get() };
		cmdList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		// Setting root signature
		cmdList->SetGraphicsRootSignature(RootSignature.Get());

		// Offset the CBV we want to use for this draw call
		cmdList->SetGraphicsRootDescriptorTable(0, CbvHeap->GetGPUDescriptorHandleForHeapStart());

		D3D12_VERTEX_BUFFER_VIEW vbv;
		vbv.BufferLocation = VertexBuffer->GetGPUVirtualAddress();
		vbv.StrideInBytes = VertexByteStride;
		vbv.SizeInBytes = VertexBufferByteSize;
		D3D12_VERTEX_BUFFER_VIEW vertexBuffers[] = { vbv };

		cmdList->IASetVertexBuffers(0, 1, vertexBuffers);


		D3D12_INDEX_BUFFER_VIEW ibv;
		ibv.BufferLocation = IndexBuffer->GetGPUVirtualAddress();
		ibv.Format = IndexFormat;
		ibv.SizeInBytes = IndexBufferByteSize;

		cmdList->IASetIndexBuffer(&ibv);
		cmdList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//cmdList->SetPipelineState(Pso.Get());

		cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
	}

	void DrawImgui()
	{
		auto commandList = Resources->GetCommandList();

		{
			static bool show_demo_window = true;
			static bool show_another_window = true;
			static float clear_color[] = { 1.f, 1.f, 1.f, 0.f };
			// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
			if (show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);

			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
				ImGui::Checkbox("Another Window", &show_another_window);

				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}

			// 3. Show another simple window.
			if (show_another_window)
			{
				ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				ImGui::Text("Hello from another window!");
				if (ImGui::Button("Close Me"))
					show_another_window = false;
				ImGui::End();
			}
		}


		ImGui::Render();

		commandList->SetDescriptorHeaps(1, ImguiHeap.GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

	}

	void Update(Transform* transform, float ratio)
	{
		
		static float red = 0;
		red += 0.3 / 60;

		// Build the view matrix.
		XMVECTOR pos = XMVectorSet(0.f, 0.f, 0.f, 1.0f);
		XMVECTOR target = XMVectorSet(0.f, 0.f, 1.f, 1.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

		XMFLOAT4X4 mView;
		XMFLOAT4X4 mProj;
		XMFLOAT4X4 ww = DirectX::XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);
		auto something = XMMatrixTranslation(0.f, 0.f, 5.f) * XMMatrixRotationY(red);

		XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
		XMStoreFloat4x4(&mView, view);

		XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * XM_PI, ratio, 0.1f, 1000.0f);
		XMStoreFloat4x4(&mProj, P);

		XMMATRIX world = XMLoadFloat4x4(&ww);
		XMMATRIX proj = XMLoadFloat4x4(&mProj);
		XMMATRIX worldViewProj = something * view * proj;

		// Update the constant buffer with the latest worldViewProj matrix.
		ObjectConstants objConstants;
		XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
		ObjectCB->CopyData(0, objConstants);
	}

	void BuildDescriptorHeaps()
	{
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = 1;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&CbvHeap));
	}

	void BuildConstantBuffers()
	{
		// Constant buffer to store the constants of n objects.
		ObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(_device, 1, true);

		UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));

		// Address to start of the buffer (0th constant buffer).
		D3D12_GPU_VIRTUAL_ADDRESS cbAddress = ObjectCB->Resource()->GetGPUVirtualAddress();

		// Offset to the ith object constant buffer in the buffer.
		int cBufIndex = 0;
		cbAddress += cBufIndex * objCBByteSize;

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(ObjectConstants));

		_device->CreateConstantBufferView(&cbvDesc, CbvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	void BuildRootSignature()
	{
		// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[1];

		// Create a single descriptor table of CBVs.
		CD3DX12_DESCRIPTOR_RANGE cbvTable;
		cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
		slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		D_HR_CHECK(hr);

		D_HR_CHECK(_device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&RootSignature)));
	}

	void BuildShadersAndInputLayout()
	{

		for (const auto& entry : std::filesystem::directory_iterator("."))

			std::string ff = entry.path().string();


		VsByteCode = CompileShader(L"..\\..\\..\\..\\..\\src\\Shaders\\SimpleColor.hlsl", nullptr, "VS", "vs_5_0");
		PsByteCode = CompileShader(L"..\\..\\..\\..\\..\\src\\Shaders\\SimpleColor.hlsl", nullptr, "PS", "ps_5_0");

		InputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}

	void BuildGeometery()
	{

		auto cmdList = Resources->GetCommandList();

		struct Vertex
		{
			Vector3 pos;
			Vector4 color;
		};

		std::array<Vertex, 8> vertices =
		{
			Vertex({ Vector3(-1.f, -1.f, -1.f), Vector4(Colors::White) }),
			Vertex({ Vector3(-1.f, +1.f, -1.f), Vector4(Colors::Black) }),
			Vertex({ Vector3(+1.f, +1.f, -1.f), Vector4(Colors::Red) }),
			Vertex({ Vector3(+1.f, -1.f, -1.f), Vector4(Colors::Green) }),
			Vertex({ Vector3(-1.f, -1.f, +1.f), Vector4(Colors::Blue) }),
			Vertex({ Vector3(-1.f, +1.f, +1.f), Vector4(Colors::Yellow) }),
			Vertex({ Vector3(+1.f, +1.f, +1.f), Vector4(Colors::Cyan) }),
			Vertex({ Vector3(+1.f, -1.f, +1.f), Vector4(Colors::Magenta) })
		};

		VertexBufferByteSize = (UINT)vertices.size() * sizeof(Vertex);
		VertexByteStride = (UINT)sizeof(Vertex);

		VertexBuffer = CreateDefaultBuffer(_device, cmdList, vertices.data(), VertexBufferByteSize, VertexUploadBuffer);

		std::array<std::uint16_t, 36> indices =
		{
			// front face
			0, 1, 2,
			0, 2, 3,

			// back face
			4, 6, 5,
			4, 7, 6,

			// left face
			4, 5, 1,
			4, 1, 0,

			// right face
			3, 2, 6,
			3, 6, 7,

			// top face
			1, 5, 6,
			1, 6, 2,

			// bottom face
			4, 0, 3,
			4, 3, 7,
		};

		IndexBufferByteSize = (UINT)indices.size() * sizeof(std::uint16_t);
		IndexBuffer = CreateDefaultBuffer(_device, cmdList, indices.data(), IndexBufferByteSize, IndexUploadBuffer);

	}

	void BuildPSO()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		psoDesc.InputLayout = { InputLayout.data(), (UINT)InputLayout.size() };
		psoDesc.pRootSignature = RootSignature.Get();
		psoDesc.VS =
		{
			reinterpret_cast<BYTE*>(VsByteCode->GetBufferPointer()),
			VsByteCode->GetBufferSize()
		};
		psoDesc.PS =
		{
			reinterpret_cast<BYTE*>(PsByteCode->GetBufferPointer()),
			PsByteCode->GetBufferSize()
		};
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = Resources->GetBackBufferFormat();
		psoDesc.SampleDesc.Count = _4xMsaaState ? 4 : 1;
		psoDesc.SampleDesc.Quality = _4xMsaaState ? (_4xMsaaQuality - 1) : 0;
		psoDesc.DSVFormat = Resources->GetDepthBufferFormat();

		_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&Pso));
	}

	void BuildImgui()
	{

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		D_HR_CHECK(_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ImguiHeap)));

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(Resources->GetWindow());
		ImGui_ImplDX12_Init(_device, Resources->GetBackBufferCount(), DXGI_FORMAT_B8G8R8A8_UNORM, ImguiHeap.Get(), ImguiHeap.Get()->GetCPUDescriptorHandleForHeapStart(), ImguiHeap.Get()->GetGPUDescriptorHandleForHeapStart());
	}

	void DisposeUploadBuffers()
	{
		VertexUploadBuffer = nullptr;
		IndexUploadBuffer = nullptr;
	}

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
	}

}