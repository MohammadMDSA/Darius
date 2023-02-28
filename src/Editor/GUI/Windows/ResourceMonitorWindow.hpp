#pragma once

#include "Window.hpp"

namespace Darius::Editor::Gui::Windows
{
	class ResourceMonitorWindow : public Window
	{
	public:
		ResourceMonitorWindow(D_SERIALIZATION::Json const& config);
		~ResourceMonitorWindow();


		// Inherited via Window
		INLINE virtual std::string GetName() const override { return "ResourceMonitor"; }

		INLINE virtual void Render(D_GRAPHICS::GraphicsContext&) override {}

		INLINE virtual void Update(float) override {}

		virtual void DrawGUI() override;

	};
}