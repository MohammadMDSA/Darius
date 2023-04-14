#pragma once

#include <rttr/type.h>

#include <functional>

#ifndef D_DETAIL_DRAWER
#define D_DETAIL_DRAWER Darius::Editor::Gui::DetailDrawer
#endif // !D_DETAIL_DRAWER

namespace Darius::Editor::Gui::DetailDrawer
{

	// Called with parameters: Name of the changed property, and the new value
	using PropertyChangeCallback = std::function<void(std::string, rttr::variant const&)>;

	void				Initialize();
	void				Shutdown();

	// Draws details of the given object with no respect of custom drawer for the root object
	bool				DefaultDetailDrawer(rttr::instance obj, PropertyChangeCallback callback = nullptr);

	// If you have registered a custom drawer and are intending to use that, this is your choice
	bool				DrawDetials(rttr::instance obj, PropertyChangeCallback callback = nullptr);

}
