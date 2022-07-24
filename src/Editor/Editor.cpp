//
// Game.cpp
//

#include "pch.hpp"
#include "Editor.hpp"
#include "Camera.hpp"
#include "GUI/GuiManager.hpp"

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include <Math/VectorMath.hpp>
#include <Core/Input.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/FrameResource.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Utils/Debug.hpp>

#include <exception>

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace D_RENDERER_FRAME_RESOUCE;
using namespace D_RENDERER_GEOMETRY;

using Microsoft::WRL::ComPtr;
namespace Darius::Editor
{

	Editor::Editor() noexcept(false)
	{
	}

	Editor::~Editor()
	{
		/*if (m_deviceResources)
		{
			m_deviceResources->WaitForGpu();
		}*/
	}

	// Initialize the Direct3D resources required to run.
	void Editor::Initialize(HWND window, int width, int height)
	{
#ifdef _DEBUG
		//D_DEBUG::AttachWinPixGpuCapturer();
#endif
		D_RENDERER_DEVICE::Initialize(window, width, height);
		D_CAMERA_MANAGER::Initialize();
		Darius::Renderer::Initialize();

		CreateDeviceDependentResources();
		CreateWindowSizeDependentResources();

		D_TIME::Initialize();
		D_INPUT::Initialize(window);

		D_GUI_MANAGER::Initialize();

		D_CAMERA_MANAGER::SetViewportDimansion((float)width, (float)height);
		D_TIME::EnableFixedTimeStep(1.0 / 60);

		mCamera = std::make_unique<D_MATH_CAMERA::Camera>();
		mCamera->SetFOV(XM_PI / 3);
		mCamera->SetZRange(0.01f, 1000.f);
		mCamera->SetPosition(Vector3(-5.f, 0.f, 0));
		mCamera->SetLookDirection(Vector3::Right(), Vector3::Up());
		mCamera->ReverseZ(false);

		D_CAMERA_MANAGER::SetActiveCamera(mCamera.get());

		D_RENDERER::RegisterGuiDrawer(&D_GUI_MANAGER::Render);
	}

#pragma region Frame Update
	// Executes the basic game loop.
	void Editor::Tick()
	{
		auto timer = D_TIME::GetStepTimer();
		timer->Tick([&]()
			{
				Update(*timer);
			});

		Render();
	}

	// Updates the world.
	void Editor::Update(D_TIME::StepTimer const& timer)
	{
		D_RENDERER_DEVICE::SyncFrame();
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

		float elapsedTime = float(timer.GetElapsedSeconds());
		(elapsedTime);
		D_INPUT::Update();

		static auto fc = FlyingFPSCamera(*mCamera, Vector3::Up());
		if (D_KEYBOARD::GetKey(D_KEYBOARD::Keys::LeftAlt) &&
			D_MOUSE::GetButton(D_MOUSE::Keys::Right))
			fc.Update(elapsedTime);
		else
			mCamera->Update();

		UpdateRotation();

		PIXEndEvent();
	}
#pragma endregion

#pragma region Frame Render
	// Draws the scene.
	void Editor::Render()
	{
		// Don't try to render anything before the first Update.
		if (D_TIME::GetStepTimer()->GetFrameCount() == 0)
		{
			return;
		}

		std::vector<RenderItem*> items;
		for (auto const& ri : mRenderItems)
		{
			items.push_back(ri.get());
		}

		// TODO: Add your rendering code here.
		Darius::Renderer::RenderMeshes(items);


	}
	void Editor::UpdateRotation()
	{
		static float red = 0;
		//red += 0.3f / 60;

		mRenderItems[0]->World = Matrix4(XMMatrixTranslation(-2.f, 1.f, -5.f));
		mRenderItems[1]->World = Matrix4(XMMatrixTranslation(2.f, -1.f, -5.f));

		// Update CBs
		std::vector<RenderItem*> renderItems;
		for (auto& ri : mRenderItems)
			renderItems.push_back(ri.get());
		D_RENDERER::UpdateMeshCBs(renderItems);
	}
#pragma endregion

#pragma region Message Handlers
	// Message handlers
	void Editor::OnActivated()
	{
		// TODO: Game is becoming active window.
	}

	void Editor::OnDeactivated()
	{
		// TODO: Game is becoming background window.
	}

	void Editor::OnSuspending()
	{
		// TODO: Game is being power-suspended (or minimized).
	}

	void Editor::OnResuming()
	{
		D_TIME::GetStepTimer()->ResetElapsedTime();

		// TODO: Game is being power-resumed (or returning from minimize).
	}

	void Editor::OnWindowMoved()
	{
		D_DEVICE::OnWindowMoved();
	}

	void Editor::OnDisplayChange()
	{
		D_DEVICE::OnDisplayChanged();
	}

	void Editor::OnWindowSizeChanged(int width, int height)
	{
		D_CAMERA_MANAGER::SetViewportDimansion((float)width, (float)height);
		if (!D_DEVICE::OnWindowsSizeChanged(width, height))
			return;

		CreateWindowSizeDependentResources();

		// TODO: Game window is being resized.
	}

	// Properties
	void Editor::GetDefaultSize(int& width, int& height) const noexcept
	{
		// TODO: Change to desired default window size (note minimum size is 320x200).
		width = 1600;
		height = 1000;
	}
#pragma endregion

#pragma region Direct3D Resources
	// These are the resources that depend on the device.
	void Editor::CreateDeviceDependentResources()
	{
		D_DEVICE::ShaderCompatibilityCheck(D3D_SHADER_MODEL_6_0);

		// If using the DirectX Tool Kit for DX12, uncomment this line:
		// m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

		InitMesh();

	}

	// Allocate all memory resources that change on a window SizeChanged event.
	void Editor::CreateWindowSizeDependentResources()
	{
		// TODO: Initialize windows-size dependent objects here.
	}

	void Editor::OnDeviceLost()
	{
		// TODO: Add Direct3D resource cleanup here.

		// If using the DirectX Tool Kit for DX12, uncomment this line:
		// m_graphicsMemory.reset();
		mCamera.reset();
	}

	void Editor::OnDeviceRestored()
	{
		CreateDeviceDependentResources();

		CreateWindowSizeDependentResources();
	}
#pragma endregion

#pragma region Mesh and Pipeline Setup
	void Editor::InitMesh()
	{

		auto cmdAlloc = D_RENDERER_DEVICE::GetCommandAllocator();
		auto cmdList = D_RENDERER_DEVICE::GetCommandList();
		auto cmdQueue = D_RENDERER_DEVICE::GetCommandQueue();

		D_HR_CHECK(cmdList->Reset(cmdAlloc, nullptr));

		BuildRootSignature();
		BuildShadersAndInputLayout();
		BuildGeometery();
		BuildRenderItems();
		//BuildDescriptorHeaps();
		//BuildConstantBuffers();
		BuildPSO();

		// Execute the initialization commands.
		D_HR_CHECK(cmdList->Close());
		ID3D12CommandList* commandLists[] = { cmdList };
		cmdQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

		// Wait until gpu executes initial commands
		D_RENDERER_DEVICE::WaitForGpu();
	}


	void Editor::BuildDescriptorHeaps()
	{
		UINT objCount = (UINT)mRenderItems.size();

		// Need a CBV descriptor for each object for each frame resource,
		// +1 for the perPass CBV for each frame resource.
		UINT numDescriptors = (objCount + 1) * gNumFrameResources;

		// Save an offset to the start of the pass CBVs.  These are the last 3 descriptors.
		D_RENDERER::PassCbvOffset = objCount * gNumFrameResources;

		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = numDescriptors;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;

		D_HR_CHECK(D_RENDERER_DEVICE::GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&D_RENDERER::CbvHeap)));
		D_RENDERER::CbvHeap = D_RENDERER::CbvHeap;
	}

	void Editor::BuildConstantBuffers()
	{
		auto device = D_RENDERER_DEVICE::GetDevice();
		UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(MeshConstants));
		UINT objCount = (UINT)mRenderItems.size();

		// Need a CBV descriptor for each object for each frame resource.
		for (int frameIndex = 0; frameIndex < gNumFrameResources; frameIndex++)
		{
			auto objectCB = D_RENDERER_DEVICE::GetFrameResourceWithIndex(frameIndex)->MeshCB->Resource();
			for (UINT i = 0; i < objCount; i++)
			{
				D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->GetGPUVirtualAddress();

				// Offset to the ith object constant buffer in the buffer.
				cbAddress += i * objCBByteSize;

				// Offset to the object cbv in the descriptor heap.
				int heapIndex = frameIndex * objCount + i;
				auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D_RENDERER::CbvHeap->GetCPUDescriptorHandleForHeapStart());
				handle.Offset(heapIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

				D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
				cbvDesc.BufferLocation = cbAddress;
				cbvDesc.SizeInBytes = objCBByteSize;

				device->CreateConstantBufferView(&cbvDesc, handle);
			}
		}

		UINT passCBByteSize = D_RENDERER_UTILS::CalcConstantBufferByteSize(sizeof(GlobalConstants));

		// Last three descriptors are the global CBVs for each frame resource.
		for (int frameIndex = 0; frameIndex < gNumFrameResources; frameIndex++)
		{
			auto globalCB = D_RENDERER_DEVICE::GetFrameResourceWithIndex(frameIndex)->GlobalCB->Resource();
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = globalCB->GetGPUVirtualAddress();

			// Offset to the pass cbv in the descriptor heap.
			int heapIndex = D_RENDERER::PassCbvOffset + frameIndex;
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D_RENDERER::CbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(heapIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = passCBByteSize;

			device->CreateConstantBufferView(&cbvDesc, handle);
		}

	}

	void Editor::BuildRootSignature()
	{
		// Root parameter can be a table, root descriptor or root constants.
		CD3DX12_ROOT_PARAMETER slotRootParameter[2];

		// Create root CBVs.
		slotRootParameter[0].InitAsConstantBufferView(0);
		slotRootParameter[1].InitAsConstantBufferView(1);

		// A root signature is an array of root parameters.
		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(2, slotRootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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

		D_HR_CHECK(D_RENDERER_DEVICE::GetDevice()->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(&D_RENDERER::RootSignature)));
	}

	void Editor::BuildShadersAndInputLayout()
	{

		for (const auto& entry : std::filesystem::directory_iterator("."))

			std::string ff = entry.path().string();


		D_RENDERER::Shaders["standardVS"] = CompileShader(L"..\\..\\..\\..\\..\\src\\Shaders\\SimpleColor.hlsl", nullptr, "VS", "vs_5_1");
		D_RENDERER::Shaders["opaquePS"] = CompileShader(L"..\\..\\..\\..\\..\\src\\Shaders\\SimpleColor.hlsl", nullptr, "PS", "ps_5_1");

		D_RENDERER::InputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}

	void Editor::BuildGeometery()
	{

		auto cmdList = D_RENDERER_DEVICE::GetCommandList();

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

		mMesh = std::make_unique<Mesh>();
		mMesh->mVertexBufferByteSize = (UINT)vertices.size() * sizeof(Vertex);
		mMesh->mVertexByteStride = (UINT)sizeof(Vertex);

		mMesh->name = "Box";

		D_HR_CHECK(D3DCreateBlob(mMesh->mVertexBufferByteSize, &mMesh->mVertexBufferCPU));
		CopyMemory(mMesh->mVertexBufferCPU->GetBufferPointer(), vertices.data(), mMesh->mVertexBufferByteSize);

		mMesh->mVertexBufferGPU = CreateDefaultBuffer(D_RENDERER_DEVICE::GetDevice(), cmdList, vertices.data(), mMesh->mVertexBufferByteSize, mMesh->mVertexBufferUploader);

		std::array<std::uint16_t, 36> indices =
		{
			// front face
			0, 2, 1,
			0, 3, 2,

			// back face
			4, 5, 6,
			4, 6, 7,

			// left face
			4, 1, 5,
			4, 0, 1,

			// right face
			3, 6, 2,
			3, 7, 6,

			// top face
			1, 6, 5,
			1, 2, 6,

			// bottom face
			4, 3, 0,
			4, 7, 3,
		};

		mMesh->mIndexBufferByteSize = (UINT)indices.size() * sizeof(std::uint16_t);


		D_HR_CHECK(D3DCreateBlob(mMesh->mIndexBufferByteSize, &mMesh->mIndexBufferCPU));
		CopyMemory(mMesh->mIndexBufferCPU->GetBufferPointer(), indices.data(), mMesh->mIndexBufferByteSize);

		mMesh->mIndexBufferGPU = CreateDefaultBuffer(D_RENDERER_DEVICE::GetDevice(), cmdList, indices.data(), mMesh->mIndexBufferByteSize, mMesh->mIndexBufferUploader);

		Mesh::Draw draw;
		draw.mBaseVertexLocation = 0;
		draw.mIndexCount = (UINT)indices.size();
		draw.mStartIndexLocation = 0;

		mMesh->mDraw.push_back(draw);
	}

	void Editor::BuildPSO()
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

		// For Opaque objects
		ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		opaquePsoDesc.InputLayout = { D_RENDERER::InputLayout.data(), (UINT)D_RENDERER::InputLayout.size() };
		opaquePsoDesc.pRootSignature = D_RENDERER::RootSignature.Get();
		opaquePsoDesc.VS =
		{
			reinterpret_cast<BYTE*>(D_RENDERER::Shaders["standardVS"]->GetBufferPointer()),
			D_RENDERER::Shaders["standardVS"]->GetBufferSize()
		};
		opaquePsoDesc.PS =
		{
			reinterpret_cast<BYTE*>(D_RENDERER::Shaders["opaquePS"]->GetBufferPointer()),
			D_RENDERER::Shaders["opaquePS"]->GetBufferSize()
		};
		opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		opaquePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		opaquePsoDesc.SampleMask = UINT_MAX;
		opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		opaquePsoDesc.NumRenderTargets = 1;
		opaquePsoDesc.RTVFormats[0] = D_RENDERER_DEVICE::GetBackBufferFormat();
		opaquePsoDesc.SampleDesc.Count = false ? 4 : 1;
		opaquePsoDesc.SampleDesc.Quality = 0;
		opaquePsoDesc.DSVFormat = D_RENDERER_DEVICE::GetDepthBufferFormat();

		D_HR_CHECK(D_RENDERER_DEVICE::GetDevice()->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&D_RENDERER::Psos["opaque"])));

		// For opaque wireframe objecs
		auto opaqueWireframePsoDesc = opaquePsoDesc;
		opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		D_HR_CHECK(D_RENDERER_DEVICE::GetDevice()->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&D_RENDERER::Psos["opaque_wireframe"])));
	}

	void Editor::BuildRenderItems()
	{
		auto box = std::make_unique<RenderItem>();
		box->World = Matrix4::Identity();
		box->Mesh = mMesh.get();
		box->ObjCBIndex = 0;
		box->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		box->IndexCount = mMesh->mDraw[0].mIndexCount;
		box->StartIndexLocation = mMesh->mDraw[0].mStartIndexLocation;
		box->BaseVertexLocation = mMesh->mDraw[0].mBaseVertexLocation;
		mRenderItems.push_back(std::move(box));

		auto box1 = std::make_unique<RenderItem>();
		box1->World = Matrix4(XMMatrixTranslation(1.f, 1.f, 0.f));
		box1->Mesh = mMesh.get();
		box1->ObjCBIndex = 1;
		box1->PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		box1->IndexCount = mMesh->mDraw[0].mIndexCount;
		box1->StartIndexLocation = mMesh->mDraw[0].mStartIndexLocation;
		box1->BaseVertexLocation = mMesh->mDraw[0].mBaseVertexLocation;
		mRenderItems.push_back(std::move(box1));

	}
#pragma endregion

}