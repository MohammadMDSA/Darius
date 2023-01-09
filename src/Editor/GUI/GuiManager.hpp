#pragma once

#include <Core/Containers/Vector.hpp>
#include <Renderer/CommandContext.hpp>
#include <Renderer/FrameResource.hpp>
#include <Scene/GameObject.hpp>

#ifndef D_GUI_MANAGER
#define D_GUI_MANAGER Darius::Editor::Gui::GuiManager
#endif // !D_GUI_MANAGER

using namespace D_GRAPHICS;

namespace Darius::Editor::Gui::GuiManager
{

	enum class Icon
	{
		Folder,
		File,

		NumIcons
	};

	void Initialize();
	void Shutdown();

	void Update(float deltaTime);
	void Render();
	void DrawGUI();
	void _DrawMenuBar();

	uint64_t GetIconTextureId(Icon iconId);
}