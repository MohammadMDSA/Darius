#pragma once

#include <Core/Containers/Vector.hpp>
#include <Renderer/CommandContext.hpp>
#include <Renderer/FrameResource.hpp>
#include <Scene/GameObject.hpp>

#ifndef D_GUI_MANAGER
#define D_GUI_MANAGER Darius::Editor::GuiManager
#endif // !D_GUI_MANAGER

using namespace D_GRAPHICS;
using namespace D_RENDERER_FRAME_RESOUCE;

namespace Darius::Editor::GuiManager
{
	void Initialize();
	void Shutdown();

	void Update(float deltaTime);
	void Render(D_GRAPHICS::GraphicsContext& context);
	void DrawGUI();
}