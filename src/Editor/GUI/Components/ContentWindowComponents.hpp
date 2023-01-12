#pragma once

#include <Core/Filesystem/Path.hpp>
#include <Utils/Common.hpp>

#ifndef D_GUI_COMPONENT
#define D_GUI_COMPONENT Darius::Editor::Gui::Component
#endif

namespace Darius::Editor::Gui::Component
{
	
	struct EditorContentWindowItem
	{
		std::string			Name;
		D_FILE::Path		Path;
		bool				IsDirectory;
	};

	void ContentWindowItemGrid(EditorContentWindowItem& data, float width, float height, _IN_OUT_ bool& selected, _OUT_ bool& doubleClicked);

}