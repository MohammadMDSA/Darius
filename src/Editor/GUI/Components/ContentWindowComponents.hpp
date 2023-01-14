#pragma once

#include <Core/Filesystem/Path.hpp>
#include <ResourceManager/Resource.hpp>
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
		uint64_t			IconId;
		D_RESOURCE::ResourceHandle Handle = D_RESOURCE::EmptyResourceHandle;
	};

	void ContentWindowItemGrid(EditorContentWindowItem& data, float width, float height, _IN_OUT_ bool& selected, _OUT_ bool& doubleClicked);

}