//
// Game.cpp
//

#include "pch.hpp"
#include "Editor.hpp"
#include "GUI/GuiManager.hpp"

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include <Math/VectorMath.hpp>
#include <Core/Input.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/FrameResource.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Renderer/GraphicsCore.hpp>
#include <Renderer/CommandContext.hpp>
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
		D_GUI_MANAGER::Shutdown();
		D_RENDERER_DEVICE::Shutdown();
	}

	// Initialize the Direct3D resources required to run.
	void Editor::Initialize(HWND window, int width, int height)
	{
#ifdef _DEBUG
		D_DEBUG::AttachWinPixGpuCapturer();
#endif
		D_RENDERER_DEVICE::Initialize(window, width, height);
		D_RENDERER::Initialize();

		CreateDeviceDependentResources();
		CreateWindowSizeDependentResources();

		D_TIME::Initialize();
		D_INPUT::Initialize(window);

		D_GUI_MANAGER::Initialize();

		D_TIME::EnableFixedTimeStep(1.0 / 60);

		D_RENDERER::RegisterGuiDrawer(&D_GUI_MANAGER::DrawGUI);
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
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

		float elapsedTime = float(timer.GetElapsedSeconds());
		(elapsedTime);
		D_INPUT::Update();

		D_GUI_MANAGER::Update(elapsedTime);

		UpdateRotation();

		PIXEndEvent();
	}
#pragma endregion

#pragma region Frame Render
	// Draws the scene.
	void Editor::Render()
	{
		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Render Scene");

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

		D_GUI_MANAGER::Render(context, items);
		Darius::Renderer::Present(context);


	}
	void Editor::UpdateRotation()
	{
		static float red = 0;
		//red += 0.3f / 60;

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
		(width);
		(height);
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

		mRenderItems[0]->World = Matrix4(XMMatrixTranslation(-2.f, 1.f, -5.f));
		mRenderItems[1]->World = Matrix4(XMMatrixTranslation(2.f, -1.f, -5.f));

		D_GUI_MANAGER::ri = mRenderItems[0].get();
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
		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Mesh Init");

		BuildRootSignature();
		BuildShadersAndInputLayout();
		BuildGeometery(context);
		BuildRenderItems();
		BuildPSO();

		context.Finish(true);
	}

	void Editor::BuildRootSignature()
	{
		// Root parameter can be a table, root descriptor or root constants.
		D_GRAPHICS_UTILS::RootParameter slotRootParameter[2];

		// A root signature is an array of root parameters.
		D_RENDERER::RootSig.Reset(2, 0);

		// Create root CBVs.
		D_RENDERER::RootSig[0].InitAsConstantBuffer(0);
		D_RENDERER::RootSig[1].InitAsConstantBuffer(1);

		D_RENDERER::RootSig.Finalize(L"Main Root Sig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
	}

	void Editor::BuildShadersAndInputLayout()
	{

		D_RENDERER::Shaders["standardVS"] = CompileShader(L"..\\..\\..\\..\\..\\src\\Shaders\\SimpleColor.hlsl", nullptr, "VS", "vs_5_1");
		D_RENDERER::Shaders["opaquePS"] = CompileShader(L"..\\..\\..\\..\\..\\src\\Shaders\\SimpleColor.hlsl", nullptr, "PS", "ps_5_1");

		D_RENDERER::InputLayout =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
		};
	}

	void Editor::BuildGeometery(D_GRAPHICS::GraphicsContext& context)
	{

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

		mMesh->mVertexBufferGPU = CreateDefaultBuffer(D_RENDERER_DEVICE::GetDevice(), context.GetCommandList(), vertices.data(), mMesh->mVertexBufferByteSize, mMesh->mVertexBufferUploader);

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

		mMesh->mIndexBufferGPU = CreateDefaultBuffer(D_RENDERER_DEVICE::GetDevice(), context.GetCommandList(), indices.data(), mMesh->mIndexBufferByteSize, mMesh->mIndexBufferUploader);

		Mesh::Draw draw;
		draw.mBaseVertexLocation = 0;
		draw.mIndexCount = (UINT)indices.size();
		draw.mStartIndexLocation = 0;

		mMesh->mDraw.push_back(draw);
	}

	void Editor::BuildPSO()
	{

		// For Opaque objects
		GraphicsPSO pso(L"Opaque");

		pso.SetInputLayout((UINT)D_RENDERER::InputLayout.size(), D_RENDERER::InputLayout.data());

		pso.SetRootSignature(D_RENDERER::RootSig);
		pso.SetVertexShader(reinterpret_cast<BYTE*>(D_RENDERER::Shaders["standardVS"]->GetBufferPointer()),
			D_RENDERER::Shaders["standardVS"]->GetBufferSize());
		pso.SetPixelShader(reinterpret_cast<BYTE*>(D_RENDERER::Shaders["opaquePS"]->GetBufferPointer()),
			D_RENDERER::Shaders["opaquePS"]->GetBufferSize());
		auto rasterState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rasterState.FillMode = D3D12_FILL_MODE_SOLID;
		pso.SetRasterizerState(rasterState);
		pso.SetBlendState(CD3DX12_BLEND_DESC(D3D12_DEFAULT));
		pso.SetDepthStencilState(CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT));
		pso.SetSampleMask(UINT_MAX);
		pso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
		pso.SetRenderTargetFormat(D_RENDERER_DEVICE::GetBackBufferFormat(), D_RENDERER_DEVICE::GetDepthBufferFormat());
		pso.Finalize();
		D_RENDERER::Psos["opaque"] = pso;


		// For opaque wireframe objecs
		GraphicsPSO wirePso(pso);
		auto wireRasterState = rasterState;
		wireRasterState.FillMode = D3D12_FILL_MODE_WIREFRAME;
		wirePso.SetRasterizerState(wireRasterState);
		wirePso.Finalize();
		D_RENDERER::Psos["opaque_wireframe"] = wirePso;
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