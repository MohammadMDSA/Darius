#include "Editor/pch.hpp"

#include "ContentWindowComponents.hpp"

#include <imgui.h>

namespace Darius::Editor::Gui::Component
{

	void ContentWindowItemGrid(EditorContentWindowItem& data, float width, float height, bool& selected, bool& doubleClicked)
	{
		auto nameStr = data.Name.c_str();
		ImGui::PushID(nameStr);

		if(data.IsDirectory)
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.6f, 0.1f, 0.1f, 1.f));

		doubleClicked = ImGui::Selectable(nameStr, &selected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(width, height));

		if (data.IsDirectory)
			ImGui::PopStyleColor();

		ImGui::PopID();
	}
}
