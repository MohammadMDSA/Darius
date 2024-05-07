//
// Game.h
//

#pragma once

#include <Core/Filesystem/Path.hpp>
#include <Core/TimeManager/StepTimer.hpp>
#include <Graphics/DeviceResources.hpp>

#include <filesystem>

#ifndef D_EDITOR
#define D_EDITOR Darius::Editor
#endif // !D_EDITOR


namespace Darius::Editor
{

	// A basic game implementation that creates a D3D12 device and
	// provides a game loop.
	class Editor final : public D_GRAPHICS_DEVICE::IDeviceNotify
	{
	public:

		Editor() noexcept(false);
		~Editor();

		Editor(Editor&&) = default;
		Editor& operator= (Editor&&) = default;

		Editor(Editor const&) = delete;
		Editor& operator= (Editor const&) = delete;

		// Initialization and management
		void Initialize(HWND window, int width, int height, D_FILE::Path const& projectPath);

		// Basic game loop
		void Tick();

		// IDeviceNotify
		void OnDeviceLost() override;
		void OnDeviceRestored() override;

		// Messages
		void OnActivated();
		void OnDeactivated();
		void OnSuspending();
		void OnResuming();
		void OnNewAudioDeviceConnected();
		void OnQuit();
		void OnWindowMoved();
		void OnDisplayChange();
		void OnWindowSizeChanged(int width, int height);

		// Properties
		void GetDefaultSize(int& width, int& height) const noexcept;

	private:

		void Update(D_TIME::StepTimer const& timer);
		void Render();

		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();

		D_TIME::StepTimer						mTimer;
	};
}