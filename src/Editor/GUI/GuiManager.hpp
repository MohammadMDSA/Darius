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

	void Initialize();
	void Shutdown();

	void Update(float deltaTime);
	void Render();
	void DrawGUI();
	void _DrawMenuBar();
	void DrawGammAddMenu(D_SCENE::GameObject* const contextGameObject);
	void SaveWindowsState();
	Darius::Editor::Gui::Windows::Window* GetWindow(std::string const& name);

	template<class WIND>
	Darius::Editor::Gui::Windows::Window* GetWindow()
	{
		return GetWindow(dynamic_cast<WIND*>(WIND::SGetName()));
	}
}