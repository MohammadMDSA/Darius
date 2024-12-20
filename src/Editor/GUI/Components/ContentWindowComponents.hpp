#pragma once

#include <Core/Containers/List.hpp>
#include <Core/Filesystem/Path.hpp>
#include <ResourceManager/Resource.hpp>
#include <Utils/Common.hpp>

#ifndef D_GUI_COMPONENT
#define D_GUI_COMPONENT Darius::Editor::Gui::Components
#endif

namespace Darius::Editor::Gui::Components
{
	
	struct EditorContentWindowItem
	{
		std::string			Name;
		D_FILE::Path		Path;
		bool				IsDirectory;
		uint64_t			IconId;
		D_RESOURCE::ResourceHandle MainHandle = D_RESOURCE::EmptyResourceHandle;
		D_CONTAINERS::DVector<D_RESOURCE::ResourceHandle> ChildResources;
	};

	void ContentWindowItemGrid(EditorContentWindowItem& data, float width, float height, bool focus, _IN_OUT_ bool& selected, _OUT_ bool& doubleClicked, D_RESOURCE::ResourceHandle& selectedResource);

}