//
// Game.cpp
//

#include "pch.hpp"
#include "Game.hpp"
#include "Math/VectorMath.hpp"

#include <Renderer/Renderer.hpp>
#include <Renderer/FrameResource.hpp>
#include <Utils/Debug.hpp>

#include <exception>

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace D_CONST_FRAME_RESOUCE;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
}

Game::~Game()
{
    /*if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }*/
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    D_DEBUG::AttachWinPixGpuCapturer();

    mWidth = (float)width;
    mHeight = (float)height;
    Darius::Renderer::Device::Initialize(window, width, height);

    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();

    Darius::Renderer::Initialize();
	InitMesh();

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    mTimer.Tick([&]()
    {
        Update(mTimer);
    });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const&)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    // float elapsedTime = float(timer.GetElapsedSeconds());
	UpdateGlobalConstants();
    UpdateRotation();

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (mTimer.GetFrameCount() == 0)
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
void Game::UpdateRotation()
{
    static float red = 0;
    //red += 0.3f / 60;

    auto ww = XMMatrixTranslation(0.f, 0.f, 5.f) * XMMatrixRotationY(red);
    for (auto& ri : mRenderItems)
        ri->World = Matrix4(ww);


    auto upBuff = D_RENDERER_DEVICE::GetCurrentFrameResource();

    // Update the constant buffer with the latest worldViewProj matrix.
    for (auto& ri : mRenderItems)
    {
        if (ri->NumFramesDirty <= 0)
            continue;
        MeshConstants objConstants;
        XMStoreFloat4x4(&objConstants.mWorld, XMMatrixTranspose(ri->World));

        upBuff->MeshCB->CopyData(ri->ObjCBIndex, objConstants);

        // Next FrameResource need to be updated too.
        ri->NumFramesDirty--;
    }
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    mTimer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    D_DEVICE::OnWindowMoved();
}

void Game::OnDisplayChange()
{
    D_DEVICE::OnDisplayChanged();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    mWidth = (float)width;
    mHeight = (float)height;

    if (!D_DEVICE::OnWindowsSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    D_DEVICE::ShaderCompatibilityCheck(D3D_SHADER_MODEL_6_0);

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    // TODO: Initialize device dependent objects here (independent of window size).
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
}

void Game::UpdateGlobalConstants()
{
	PIXBeginEvent(PIX_COLOR_DEFAULT, "Update Globals");
    D_CONST_FRAME_RESOUCE::GlobalConstants globals;

    // Build the view matrix.
    Vector3 pos = Vector3(0.f, 0.f, 0.f);
    Vector3 target = Vector3(0.f, 0.f, 1.f);
    Vector3 up = Vector3(0.0f, 1.0f, 0.0f);
    float nearClip = 0.01f;
    float farClip = 1000.f;

    auto view = Matrix4(XMMatrixLookAtLH(pos, target, up));
    auto proj = Matrix4(XMMatrixPerspectiveFovLH(XM_PI * 0.25f, mWidth / mHeight, nearClip, farClip));

    auto viewProj = view * proj;
    auto invView = Matrix4::Inverse(view);
    auto invProj = Matrix4::Inverse(proj);
    auto invViewProj = Matrix4::Inverse(viewProj);

    globals.View = view.Transpose();
    globals.InvView = invView.Transpose();
    globals.Proj = proj.Transpose();
    globals.InvProj = invProj.Transpose();
    globals.ViewProj = viewProj.Transpose();
    globals.InvViewProj = invViewProj.Transpose();
    globals.CameraPos = pos;
    globals.RenderTargetSize = XMFLOAT2(mWidth, mHeight);
    globals.InvRenderTargetSize = XMFLOAT2(1.f / mWidth, 1.f / mHeight);
    globals.NearZ = nearClip;
    globals.FarZ = farClip;
    globals.TotalTime = (float)mTimer.GetTotalSeconds();
    globals.DeltaTime = (float)mTimer.GetElapsedSeconds();

    auto currGlobalCb = D_RENDERER_DEVICE::GetCurrentFrameResource()->GlobalCB.get();

    currGlobalCb->CopyData(0, globals);
	PIXEndEvent();

}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.

    // If using the DirectX Tool Kit for DX12, uncomment this line:
    // m_graphicsMemory.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

void Game::InitMesh()
{

    auto cmdAlloc = D_RENDERER_DEVICE::GetCommandAllocator();
    auto cmdList = D_RENDERER_DEVICE::GetCommandList();
    auto cmdQueue = D_RENDERER_DEVICE::GetCommandQueue();

    D_HR_CHECK(cmdAlloc->Reset());
    D_HR_CHECK(cmdList->Reset(cmdAlloc, nullptr));

    BuildRootSignature();
    BuildShadersAndInputLayout();
    BuildGeometery();
	BuildRenderItems();
    BuildDescriptorHeaps();
    BuildConstantBuffers();
    BuildPSO();
    BuildImgui();

    // Execute the initialization commands.
    D_HR_CHECK(cmdList->Close());
    ID3D12CommandList* commandLists[] = { cmdList };
    cmdQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

    // Wait until gpu executes initial commands
    D_RENDERER_DEVICE::WaitForGpu();
}


void Game::BuildDescriptorHeaps()
{
	UINT objCount = (UINT)mRenderItems.size();

	// Need a CBV descriptor for each object for each frame resource,
	// +1 for the perPass CBV for each frame resource.
	UINT numDescriptors = (objCount + 1) * gNumFrameResources;

	// Save an offset to the start of the pass CBVs.  These are the last 3 descriptors.
	D_RENDERER_DEVICE::PassCbvOffset = objCount * gNumFrameResources;

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = numDescriptors;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;

	D_HR_CHECK(D_RENDERER_DEVICE::GetDevice()->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&D_RENDERER_DEVICE::CbvHeap)));
	D_RENDERER_DEVICE::CbvHeap = D_RENDERER_DEVICE::CbvHeap;
}

void Game::BuildConstantBuffers()
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
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D_RENDERER_DEVICE::CbvHeap->GetCPUDescriptorHandleForHeapStart());
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
		int heapIndex = D_RENDERER_DEVICE::PassCbvOffset + frameIndex;
		auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(D_RENDERER_DEVICE::CbvHeap->GetCPUDescriptorHandleForHeapStart());
		handle.Offset(heapIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
		cbvDesc.BufferLocation = cbAddress;
		cbvDesc.SizeInBytes = passCBByteSize;

		device->CreateConstantBufferView(&cbvDesc, handle);
	}

}

void Game::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE cbvTable0;
	cbvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);

	CD3DX12_DESCRIPTOR_RANGE cbvTable1;
	cbvTable1.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];

	// Create root CBVs.
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable0);
	slotRootParameter[1].InitAsDescriptorTable(1, &cbvTable1);

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
		IID_PPV_ARGS(&D_RENDERER_DEVICE::RootSignature)));
}

void Game::BuildShadersAndInputLayout()
{

	for (const auto& entry : std::filesystem::directory_iterator("."))

		std::string ff = entry.path().string();


	D_RENDERER_DEVICE::Shaders["standardVS"] = CompileShader(L"..\\..\\..\\..\\..\\src\\Shaders\\SimpleColor.hlsl", nullptr, "VS", "vs_5_1");
	D_RENDERER_DEVICE::Shaders["opaquePS"] = CompileShader(L"..\\..\\..\\..\\..\\src\\Shaders\\SimpleColor.hlsl", nullptr, "PS", "ps_5_1");

	D_RENDERER_DEVICE::InputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}

void Game::BuildGeometery()
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

void Game::BuildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	// For Opaque objects
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	opaquePsoDesc.InputLayout = { D_RENDERER_DEVICE::InputLayout.data(), (UINT)D_RENDERER_DEVICE::InputLayout.size() };
	opaquePsoDesc.pRootSignature = D_RENDERER_DEVICE::RootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(D_RENDERER_DEVICE::Shaders["standardVS"]->GetBufferPointer()),
		D_RENDERER_DEVICE::Shaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(D_RENDERER_DEVICE::Shaders["opaquePS"]->GetBufferPointer()),
		D_RENDERER_DEVICE::Shaders["opaquePS"]->GetBufferSize()
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

	D_HR_CHECK(D_RENDERER_DEVICE::GetDevice()->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&D_RENDERER_DEVICE::Psos["opaque"])));

	// For opaque wireframe objecs
	auto opaqueWireframePsoDesc = opaquePsoDesc;
	opaqueWireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	D_HR_CHECK(D_RENDERER_DEVICE::GetDevice()->CreateGraphicsPipelineState(&opaqueWireframePsoDesc, IID_PPV_ARGS(&D_RENDERER_DEVICE::Psos["opaque_wireframe"])));
}

void Game::BuildRenderItems()
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

void Game::BuildImgui()
{

	/*D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	D_HR_CHECK(D_RENDERER_DEVICE::GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ImguiHeap)));

	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Resources->GetWindow());
	ImGui_ImplDX12_Init(D_RENDERER_DEVICE::GetDevice(), Resources->GetBackBufferCount(), DXGI_FORMAT_B8G8R8A8_UNORM, ImguiHeap.Get(), ImguiHeap.Get()->GetCPUDescriptorHandleForHeapStart(), ImguiHeap.Get()->GetGPUDescriptorHandleForHeapStart());*/
}
