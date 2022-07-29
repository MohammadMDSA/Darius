#pragma once

#include <Renderer/CommandContext.hpp>
#include <Renderer/FrameResource.hpp>

#ifndef D_GUI_MANAGER
#define D_GUI_MANAGER Darius::Editor::GuiManager
#endif // !D_GUI_MANAGER

using namespace D_GRAPHICS;
using namespace D_RENDERER_FRAME_RESOUCE;

namespace Darius::Editor::GuiManager
{
	extern RenderItem* ri;

	void Initialize();
	void Shutdown();

	void Update(float deltaTime);
	void Render(D_GRAPHICS::GraphicsContext& context, std::vector<RenderItem*> const& renderItems);
	void DrawGUI();
}