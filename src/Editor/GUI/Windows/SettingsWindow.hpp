#pragma once

#include "Window.hpp"

namespace Darius::Editor::Gui::Windows
{
	class SettingsWindow : public Window
	{
	public:
		SettingsWindow();
		~SettingsWindow();

		// Inherited via Window
		INLINE virtual std::string const GetName() override { return "Content"; }

		INLINE virtual void			Render(D_GRAPHICS::GraphicsContext&) override {}

		INLINE virtual void			Update(float) override {}

		virtual void				DrawGUI() override;

	private:

	};
}
