#include "pch.hpp"
#include "Input.hpp"

#include <Utils/Assert.hpp>

using namespace DirectX;

namespace Darius::InputManager
{
	std::unique_ptr<DirectX::Keyboard>				_keyboard = nullptr;
	DirectX::Keyboard::KeyboardStateTracker			tracker;
	DirectX::Keyboard::State						currentState;

	void Initialize()
	{
		D_ASSERT(_keyboard == nullptr);
		_keyboard = std::make_unique<DirectX::Keyboard>();
	}

	void Shutdown()
	{
		D_ASSERT(_keyboard != nullptr);
		_keyboard.release();
	}

	void Update()
	{
		currentState = _keyboard->GetState();
		tracker.Update(currentState);
	}

	void _processMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
	}

	namespace Keyboard
	{
		bool GetKey(Keys key)
		{
			return currentState.IsKeyDown(key);
		}

		bool IsKeyDown(Keys key)
		{
			return tracker.IsKeyPressed(key);
		}

		bool IsKeyUp(Keys key)
		{
			return tracker.IsKeyReleased(key);
		}
	}
}