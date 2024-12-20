//
// Game.cpp
//

#include "pch.hpp"
#include "Editor.hpp"

#include "EditorContext.hpp"
#include "GUI/GuiManager.hpp"
#include "GUI/GuiRenderer.hpp"

#include <Core/Containers/Vector.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Engine/EngineContext.hpp>
#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Renderer/Rasterization/Renderer.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
#include <Utils/Debug.hpp>

#include <exception>

extern void ExitGame() noexcept;

using namespace DirectX;
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
		D_EDITOR_CONTEXT::Shutdown();

		mGame->Shutdown();

		// Shuting down engine
		D_ENGINE_CONTEXT::Shutdown();
	}

	// Initialize the Direct3D resources required to run.
	void Editor::Initialize(D_APP::GameProject* game, HWND window, int width, int height, D_FILE::Path const& projectPath)
	{
		mGame = game;

#ifdef _DEBUG
		// D_DEBUG::AttachWinPixGpuCapturer();
#endif

		// Initializing engine
		D_ENGINE_CONTEXT::Initialize(projectPath, window, width, height);

		// Initialize game project
		game->Initialize();

		// Initializing the editor context manager
		D_EDITOR_CONTEXT::Initialize(window);

		// Setting V-Sync
		D_TIME::EnableFixedTimeStep(1.0 / 60);

		mTimer.SetFixedTimeStep(false);
		mTimer.SetTargetElapsedSeconds(1.f / 60.f);
		mTimer.Resume();
	}

#pragma region Frame Update
	// Executes the basic game loop.
	void Editor::Tick()
	{
		mTimer.Tick([&]()
			{
				D_PROFILING::Update();
				Update(mTimer);
				Render();
				D_PROFILING::FinishFrame();
			});
	}

	// Updates the world.
	void Editor::Update(D_TIME::StepTimer const& timer)
	{
		D_PROFILING::ScopedTimer _profiler(L"Update CPU");

		float elapsedTime = float(timer.GetElapsedSeconds());

		D_EDITOR_CONTEXT::Update(elapsedTime);

		D_RESOURCE::UpdateGPUResources();
	}
#pragma endregion

#pragma region Frame Render
	// Draws the scene.
	void Editor::Render()
	{
		//D_PROFILING::ScopedTimer _prof(L"Render");

		// Don't try to render anything before the first Update.
		if (mTimer.GetFrameCount() < 2)
		{
			return;
		}

		D_GUI_MANAGER::Render();		// Rendering each window
		D_GUI_RENDERER::Render();		// Rendering editor gui
		D_GRAPHICS::Present();			// Present the frame

	}
#pragma endregion

#pragma region Message Handlers
	// Message handlers
	void Editor::OnActivated()
	{
		D_APP::_AppActivated();
	}

	void Editor::OnDeactivated()
	{
		D_APP::_AppDeactivated();
	}

	void Editor::OnSuspending()
	{
		D_APP::_AppSuspended();
	}

	void Editor::OnNewAudioDeviceConnected()
	{
		D_APP::_NewAudioDeviceConnected();
	}

	void Editor::OnResuming()
	{
		D_TIME::GetStepTimer()->ResetElapsedTime();

		D_APP::_AppResuming();
	}

	void Editor::OnQuit()
	{
		D_APP::_AppQuitting();
	}

	void Editor::OnWindowMoved()
	{
		D_GRAPHICS_DEVICE::OnWindowMoved();
	}

	void Editor::OnDisplayChange()
	{
		D_GRAPHICS_DEVICE::OnDisplayChanged();
	}

	void Editor::OnWindowSizeChanged(int width, int height)
	{
		if (!D_GRAPHICS_DEVICE::OnWindowsSizeChanged(width, height))
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