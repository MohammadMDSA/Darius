#pragma once

#include "Window.hpp"

namespace Darius::Editor::Gui::Windows
{
	class ResourceMonitorWindow : public Window
	{
	public:
		ResourceMonitorWindow();
		~ResourceMonitorWindow();


		// Inherited via Window
		INLINE virtual std::string const GetName() override { return "ResourceMonitor"; }

		INLINE virtual void Render(D_GRAPHICS::GraphicsContext&) override {}

		INLINE virtual void Update(float) override {}

		virtual void DrawGUI() override;

	};
}