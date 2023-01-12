#pragma once

#include "Window.hpp"

namespace Darius::Editor::Gui::Windows
{
	class DetailsWindow : public Window
	{
	public:
		DetailsWindow();
		~DetailsWindow();

		// Inherited via Window
		INLINE virtual std::string const GetName() override { return "Details"; }

		INLINE virtual void Render(D_GRAPHICS::GraphicsContext&) override {}

		INLINE virtual void Update(float) override {}

		virtual void DrawGUI() override;

	private:


	};

}