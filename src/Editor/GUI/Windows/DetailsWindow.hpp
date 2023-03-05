#pragma once

#include "Window.hpp"

namespace Darius::Editor::Gui::Windows
{
	class DetailsWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY(DetailsWindow, "Details");

	public:
		// Inherited via Window

		INLINE virtual void Render(D_GRAPHICS::GraphicsContext&) override {}

		INLINE virtual void Update(float) override {}

		virtual void DrawGUI() override;

	private:


	};

}