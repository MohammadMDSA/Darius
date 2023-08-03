#pragma once

#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>

#ifndef D_GUI_RENDERER
#define D_GUI_RENDERER Darius::Editor::Gui::Renderer
#endif // !D_GUI_RENDERER

namespace Darius::Editor::Gui::Renderer
{
	void					Initialize();
	void					Shutdown();

	D_GRAPHICS_MEMORY::DescriptorHandle AllocateUiTexture(UINT count = 1);
	void					Render();
}