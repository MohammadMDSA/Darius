#pragma once

#include <Core/Serialization/Json.hpp>
#include <Utils/Common.hpp>

#include "Libs/DirectXTK12/Inc/Keyboard.h"

#ifndef D_INPUT
#define D_INPUT Darius::InputManager
#endif // !D_INPUT

#ifndef D_MOUSE
#define D_MOUSE Darius::InputManager::Mouse
#endif // !D_INPUT

#ifndef D_KEYBOARD
#define D_KEYBOARD Darius::InputManager::Keyboard
#endif // !D_INPUT

namespace Darius::InputManager
{
	void Initialize(HWND window, D_SERIALIZATION::Json const& settings);
	void Shutdown();
#ifdef _D_EDITOR
	bool OptionsDrawer(D_SERIALIZATION::Json&);
#endif


	void Update();

	void _processKeyboardMessage(UINT message, WPARAM wParam, LPARAM lParam);
	void _processMouseMessage(UINT message, WPARAM wParam, LPARAM lParam);

	namespace Keyboard
	{
		using Keys = DirectX::Keyboard::Keys;

		bool GetKey(Keys key);
		bool IsKeyDown(Keys key);
		bool IsKeyUp(Keys key);
	}

	namespace Mouse
	{
		enum class Keys : unsigned char
		{
			Right,
			Middle,
			Left
		};

		enum class Axis : unsigned char
		{
			Horizontal,
			Vertical
		};

		void GetPosInPix(int& x, int& y);
		int GetMovement(Axis axis);
		bool GetButton(Keys key);
		bool GetButtonUp(Keys key);
		bool GetButtonDown(Keys key);
		int GetWheel();
		void SetVisibility(bool visible);
		bool IsVisible();
	}
}