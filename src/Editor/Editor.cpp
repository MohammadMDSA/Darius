//
// Game.cpp
//

#include "pch.hpp"
#include "Editor.hpp"
#include "Gui/GuiManager.hpp"
#include "EditorContext.hpp"
#include "Simulation.hpp"

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
		D_SIMULATE::Shutdown();
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

		// Creating device and window resources
		CreateDeviceDependentResources();
		CreateWindowSizeDependentResources();

		// Initialing the tiem manager
		D_TIME::Initialize();

		// Initializing the input manater
		D_INPUT::Initialize(window);

		// Initializing the resource manager
		D_RESOURCE::Initialize();

		// Initializing the editor context manager
		D_EDITOR_CONTEXT::Initialize(projectPath);

		// Initializing the simulator
		D_SIMULATE::Initialize();

		// Setting V-Sync
		D_TIME::EnableFixedTimeStep(1.0 / 60);

		// Registering gui drawer function
		D_RENDERER::RegisterGuiDrawer(&D_GUI_MANAGER::DrawGUI);

		// Registering components
		// TODO:: Better component initialization
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

		// Updating the simulator
		D_SIMULATE::Update(elapsedTime);

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