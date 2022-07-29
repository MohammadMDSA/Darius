#pragma once

#include <Renderer/CommandContext.hpp>

#ifndef D_GUI_MANAGER
#define D_GUI_MANAGER Darius::Editor::GuiManager
#endif // !D_GUI_MANAGER

using namespace D_GRAPHICS;

namespace Darius::Editor::GuiManager
{
	void Initialize();
	void Shutdown();

	void Update(float deltaTime);
	void Render(D_GRAPHICS::GraphicsContext& context);
	void DrawGUI();
}