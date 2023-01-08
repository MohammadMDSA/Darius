#pragma once

#include "Window.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Vector.hpp>

namespace Darius::Editor::Gui::Windows
{
	class ContentWindow : public Window
	{
	public:
		ContentWindow();
		~ContentWindow();

		// Inherited via Window
		INLINE virtual std::string const GetName() override { return "Profiler"; }

		INLINE virtual void Render(D_GRAPHICS::GraphicsContext&) override {}

		INLINE virtual void Update(float) override {}

		virtual void DrawGUI() override;

		D_CH_R_FIELD(D_FILE::Path, CurrentDirectory);
	private:
		
		D_CONTAINERS::DVector<std::pair<bool, std::string>>	mCurrentDirectoryItems; // isDirectory, name

	};
}
