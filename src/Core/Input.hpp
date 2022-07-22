#pragma once

#ifndef D_INPUT
#define D_INPUT Darius::InputManager
#endif // !D_INPUT

#ifndef D_MOUSE
#define D_INPUT Darius::InputManager::Mouse
#endif // !D_INPUT

#ifndef D_KEYBOARD
#define D_INPUT Darius::InputManager::Keyboard
#endif // !D_INPUT


namespace Darius::InputManager
{
	void Initialize();
	void Shutdown();
	void Update();

	void _processMessage(UINT message, WPARAM wParam, LPARAM lParam);

	namespace Keyboard
	{
		using Keys = DirectX::Keyboard::Keys;

		bool GetKey(Keys key);
		bool IsKeyDown(Keys key);
		bool IsKeyUp(Keys key);
	}

	namespace Mouse
	{

	}
}