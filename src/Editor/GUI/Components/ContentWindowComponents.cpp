#include "Editor/pch.hpp"

#include "ContentWindowComponents.hpp"

#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>

namespace Darius::Editor::Gui::Component
{

	void ContentWindowItemGrid(EditorContentWindowItem& data, float width, float height, bool& selected, bool& doubleClicked)
	{

		auto pathStr = data.Path.c_str();
		ImGui::PushID(pathStr);

		ImGui::BeginChildFrame(ImGui::GetID(pathStr), ImVec2(width, height), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground);

		auto availWidth = ImGui::GetContentRegionAvail().x;

		auto textHight = ImGui::GetTextLineHeight();
		auto padding = textHight + 5.f;
		auto size = ImVec2(width - 2 * padding, height - 2 * padding);

		auto startCurPos = ImGui::GetCursorPos();

		selected = ImGui::Button("##contentElBtn", ImVec2(-1, -1));

		// Drag and drop
		if (!data.IsDirectory)
		{
			if (data.Handle.IsValid() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptBeforeDelivery))
			{
				D_RESOURCE::ResourceDragDropPayloadContent payload;
				payload.Handle = data.Handle;
				payload.Type = std::to_string(data.Handle.Type);

				ImGui::SetDragDropPayload(D_PAYLOAD_TYPE_RESOURCE, &payload, sizeof(D_RESOURCE::ResourceDragDropPayloadContent), ImGuiCond_Once);
				ImGui::Text((data.Name + " (Resource)").c_str());
				ImGui::EndDragDropSource();
			}

		}

		doubleClicked = ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered();

		if (ImGui::IsItemHovered())
			ImGui::SetTooltip(data.Path.filename().string().c_str());

		ImGui::SetCursorPos(ImVec2((availWidth - size.x) / 2 + startCurPos.x, startCurPos.y + 5));

		ImGui::Image((ImTextureID)data.IconId, size);

		auto nameStr = data.Name.c_str();

		auto textWidth = ImGui::CalcTextSize(nameStr, 0, 0, availWidth).x;

		auto textStart = (availWidth - textWidth) / 2;
		textStart = textStart > 0 ? textStart : 0;

		ImGui::SetCursorPos(ImVec2(startCurPos.x + textStart, startCurPos.y + height - 1.5f * padding));

		ImGui::TextWrapped(nameStr);

		ImGui::EndChildFrame();

		ImGui::PopID();
	}
}
