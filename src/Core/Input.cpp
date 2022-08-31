#include "pch.hpp"
#include "Input.hpp"

#include <Utils/Assert.hpp>

#include <Mouse.h>


using namespace DirectX;

namespace Darius::InputManager
{
	std::unique_ptr<DirectX::Keyboard>				_keyboard = nullptr;
	std::unique_ptr<DirectX::Mouse>					_mouse = nullptr;
	DirectX::Keyboard::KeyboardStateTracker			keyboardTracker;
	DirectX::Keyboard::State						currentKeyboardState;
	DirectX::Mouse::ButtonStateTracker				mouseTracker;
	DirectX::Mouse::State							currentMouseState;

	int												lastX;
	int												lastY;
	int												lastWheel;
	int												currentX;
	int												currentY;
	int												currentWheel;

	void Initialize(HWND window)
	{
		D_ASSERT(_keyboard == nullptr);
		D_ASSERT(_mouse == nullptr);
		_keyboard = std::make_unique<DirectX::Keyboard>();
		_mouse = std::make_unique<DirectX::Mouse>();
		_mouse->SetWindow(window);
	}

	void Shutdown()
	{
		D_ASSERT(_keyboard != nullptr);
		D_ASSERT(_mouse != nullptr);
		_keyboard.release();
		_mouse.release();
	}

	void Update()
	{
		currentKeyboardState = _keyboard->GetState();
		keyboardTracker.Update(currentKeyboardState);

		currentMouseState = _mouse->GetState();
		mouseTracker.Update(currentMouseState);
		lastX = currentX;
		lastY = currentY;
		lastWheel = currentWheel;
		currentX = currentMouseState.x;
		currentY = currentMouseState.y;
		currentWheel = currentMouseState.scrollWheelValue;
	}

	void _processKeyboardMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		DirectX::Keyboard::ProcessMessage(message, wParam, lParam);
	}

	void _processMouseMessage(UINT message, WPARAM wParam, LPARAM lParam)
	{
		DirectX::Mouse::ProcessMessage(message, wParam, lParam);
	}

	namespace Keyboard
	{
		using Keys = DirectX::Keyboard::Keys;

		bool GetKey(Keys key)
		{
			return currentKeyboardState.IsKeyDown(key);
		}

		bool IsKeyDown(Keys key)
		{
			return keyboardTracker.IsKeyPressed(key);
		}

		bool IsKeyUp(Keys key)
		{
			return keyboardTracker.IsKeyReleased(key);
		}
	}

	namespace Mouse
	{
		void GetPosInPix(int& x, int& y)
		{
			x = currentMouseState.x;
			y = currentMouseState.y;
		}

		int GetMovement(Axis axis)
		{
			switch (axis)
			{
			case Axis::Horizontal:
				return currentX - lastX;
			case Axis::Vertical:
				return currentY - lastY;
			default:
				return 0;
			}
		}

		bool GetButton(Keys key)
		{
			switch (key)
			{
			case Keys::Right:
				return currentMouseState.rightButton;
			case Keys::Middle:
				return currentMouseState.middleButton;
			case Keys::Left:
				return currentMouseState.leftButton;
			default:
				return false;
			}
		}

		bool GetButtonUp(Keys key)
		{
			switch (key)
			{
			case Keys::Right:
				return mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
			case Keys::Middle:
				return mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
			case Keys::Left:
				return mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::RELEASED;
			default:
				return false;
			}
		}

		bool GetButtonDown(Keys key)
		{
			switch (key)
			{
			case Keys::Right:
				return mouseTracker.rightButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
			case Keys::Middle:
				return mouseTracker.middleButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
			case Keys::Left:
				return mouseTracker.leftButton == DirectX::Mouse::ButtonStateTracker::PRESSED;
			default:
				return false;
			}
		}

		int GetWheel()
		{
			return currentWheel - lastWheel;
		}

		void SetVisibility(bool visible)
		{
			_mouse->SetVisible(visible);
		}

		bool IsVisible()
		{
			return _mouse->IsVisible();
		}
	}

}