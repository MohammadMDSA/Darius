#pragma once

#ifndef D_GUI_MANAGER
#define D_GUI_MANAGER Darius::Editor::GuiManager
#endif // !D_GUI_MANAGER

namespace Darius::Editor::GuiManager
{
	void Initialize();
	void Shutdown();

	void Render();
}