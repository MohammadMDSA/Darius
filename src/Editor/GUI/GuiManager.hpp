#pragma once

#include <Core/Containers/Vector.hpp>
#include <Graphics/CommandContext.hpp>
#include <Scene/GameObject.hpp>

#ifndef D_GUI_MANAGER
#define D_GUI_MANAGER Darius::Editor::Gui::GuiManager
#endif // !D_GUI_MANAGER

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
}