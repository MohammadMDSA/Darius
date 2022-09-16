//
// Game.cpp
//

#include "pch.hpp"
#include "Editor.hpp"
#include "Gui/GuiManager.hpp"
#include "EditorContext.hpp"

#include "imgui_impl_dx12.h"
#include "imgui_impl_win32.h"

#include <Math/VectorMath.hpp>
#include <Core/Input.hpp>
#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/TimeManager/TimeManager.hpp>

#include <Scene/EntityComponentSystem/Components/MeshRendererComponent.hpp>
#include <Scene/EntityComponentSystem/Components/LightComponent.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include <Renderer/Renderer.hpp>
#include <Renderer/FrameResource.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Renderer/GraphicsCore.hpp>
#include <Renderer/CommandContext.hpp>
#include <Renderer/GraphicsUtils/Profiling/Profiling.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Debug.hpp>

#include <exception>

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace D_RENDERER_FRAME_RESOUCE;
using namespace D_RENDERER_GEOMETRY;
using namespace D_CONTAINERS;

using Microsoft::WRL::ComPtr;
namespace Darius::Editor
{

	Editor::Editor() noexcept(false)
	{
	}

	Editor::~Editor()
	{
		D_WORLD::Shutdown();
		D_EDITOR_CONTEXT::Shutdown();
		D_RESOURCE::Shutdown();
		D_INPUT::Shutdown();
		D_TIME::Shutdown();
		D_RENDERER::Shutdown();
		D_RENDERER_DEVICE::Shutdown();
	}

	// Initialize the Direct3D resources required to run.
	void Editor::Initialize(HWND window, int width, int height, D_FILE::Path projectPath)
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
		D_RESOURCE::Initialize();

		D_EDITOR_CONTEXT::Initialize(projectPath);

		D_WORLD::Initialize();

		D_TIME::EnableFixedTimeStep(1.0 / 60);

		D_RENDERER::RegisterGuiDrawer(&D_GUI_MANAGER::DrawGUI);

		/*for (size_t i = 0; i < 100; i++)
		{
			auto ob = D_WORLD::CreateGameObject();
			ob->SetMesh(D_RESOURCE::GetDefaultResource(D_RESOURCE::DefaultResource::SphereMesh));
			auto x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			auto y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			auto z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

			x = (x * 2 - 1) * 100.f;
			y = (y * 2 - 1) * 100.f;
			z = (z * 2 - 1) * 100.f;
			ob->mTransform = Transform(Vector3(x, y, z));

		}
		a1->SetMesh(D_RESOURCE::GetDefaultResource(D_RESOURCE::DefaultResource::SphereMesh));
		a2->SetMesh(D_RESOURCE::GetDefaultResource(D_RESOURCE::DefaultResource::SphereMesh));
		a1->mTransform = Transform(Vector3(-2.f, 1.f, -5.f));
		a2->mTransform = Transform(Vector3(2.f, -1.f, -5.f));
		D_EDITOR_CONTEXT::SetSelectedGameObject(a2);*/
		D_ECS_COMP::LightComponent::StaticConstructor();
		D_ECS_COMP::MeshRendererComponent::StaticConstructor();
		D_ECS_COMP::TransformComponent::StaticConstructor();
	}

#pragma region Frame Update
	// Executes the basic game loop.
	void Editor::Tick()
	{
		auto timer = D_TIME::GetStepTimer();
		timer->Tick([&]()
			{
				D_PROFILING::Update();
				Update(*timer);
			});
		Render();

	}

	// Updates the world.
	void Editor::Update(D_TIME::StepTimer const& timer)
	{
		PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

		float elapsedTime = float(timer.GetElapsedSeconds());
		D_INPUT::Update();
		D_WORLD::Update(elapsedTime);

		auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Update resources");
		D_LIGHT::UpdateBuffers(context);
		D_RESOURCE::UpdateGPUResources(context);
		context.Finish();

		D_GUI_MANAGER::Update(elapsedTime);

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

		D_GUI_MANAGER::Render(context);
		Darius::Renderer::Present(context);


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

}