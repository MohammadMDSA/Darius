#pragma once


#ifndef D_GUI_COMPONENT
#define D_GUI_COMPONENT Darius::Editor::Gui::Components
#endif

namespace Darius::Editor::Gui::Components
{

    bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);

}
