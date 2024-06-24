#pragma once

#include <Core/Containers/Vector.hpp>
#include <Graphics/CommandContext.hpp>
#include <Scene/GameObject.hpp>

#ifndef D_GUI_MANAGER
#define D_GUI_MANAGER Darius::Editor::Gui::GuiManager
#endif // !D_GUI_MANAGER

namespace Darius::Editor::Gui::Windows
{
	class Window;
}

namespace Darius::Editor::Gui::GuiManager
{

	void Initialize(HWND window);
	void Shutdown();

	void Update(float deltaTime);
	void Render();
	void DrawGUI();
	void _DrawMenuBar();
	void DrawGammAddMenu(D_SCENE::GameObject* const contextGameObject);
	void SaveWindowsState();
	Darius::Editor::Gui::Windows::Window* GetWindow(std::string const& name);
	void MapWindowPointToScreen(POINT const& windowSpace, POINT& screenSpace);
	void MapWindowRectToScreen(RECT const& windowSpace, RECT& screenSpace);

	template<class WIND>
	WIND* GetWindow()
	{
		return dynamic_cast<WIND*>(GetWindow(WIND::SGetName()));
	}
}