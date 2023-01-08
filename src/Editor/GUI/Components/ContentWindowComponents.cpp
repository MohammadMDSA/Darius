#include "Editor/pch.hpp"

#include "ContentWindowComponents.hpp"
#include "Editor/GUI/GuiManager.hpp"

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

namespace Darius::Editor::Gui::Component
{

	void ContentWindowItemGrid(EditorContentWindowItem& data, float width, float height, bool& selected, bool& doubleClicked)
	{

		auto nameStr = data.Name.c_str();
		ImGui::PushID(nameStr);

		ImGui::BeginGroup();

		auto padding = 15.f;
		auto paddingVec = ImVec2(padding, padding);
		auto size = ImVec2(width - 2 * padding, height - 2 * padding);

		auto startCurPos = ImGui::GetCursorPos();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, paddingVec);

		if (data.IsDirectory)
			ImGui::ImageButton("##icon", (ImTextureID)D_GUI_MANAGER::GetIconTextureId(D_GUI_MANAGER::Icon::Folder), size);

		else
			ImGui::ImageButton("##icon", (ImTextureID)D_GUI_MANAGER::GetIconTextureId(D_GUI_MANAGER::Icon::File), size);

		ImGui::PopStyleVar();

		ImGui::SetCursorPos(ImVec2(startCurPos.x, startCurPos.y + height - padding + 2));
		
		//ImGui::TextWrapped(data.Name.c_str(), );

		selected = false;
		doubleClicked = ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered();

		ImGui::EndGroup();

		ImGui::PopID();
	}
}
