#pragma once

#include "Window.hpp"

namespace Darius::Editor::Gui::Windows
{
	class ResourceMonitorWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY(ResourceMonitorWindow, "Resource Monitor");
	public:

		// Inherited via Window

		INLINE virtual void Update(float) override {}

		virtual void DrawGUI() override;

	};
}