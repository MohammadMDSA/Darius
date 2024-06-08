#include "Editor/pch.hpp"

#include "ContentWindowComponents.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/Resource.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>
#include <Scene/Resources/PrefabResource.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

namespace Darius::Editor::Gui::Components
{

	void ContentWindowItemGrid(EditorContentWindowItem& data, float width, float height, bool focus, bool& selected, bool& doubleClicked, D_RESOURCE::ResourceHandle& selectedResource)
	{

		auto pathStr = data.Path.c_str();
		ImGui::PushID(pathStr);

		ImGui::BeginChildFrame(ImGui::GetID(pathStr), ImVec2(width, height), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground);

		auto availWidth = ImGui::GetContentRegionAvail().x;

		auto textHight = ImGui::GetTextLineHeight();
		auto padding = textHight + 5.f;
		auto size = ImVec2(width - 2 * padding, height - 2 * padding);

		auto startCurPos = ImGui::GetCursorPos();

		if(focus)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 1.f, 0.f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 1.f, 0.f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, 1.f, 0.f, 1.f));
		}

		selected = ImGui::Button("##contentElBtn", ImVec2(-1, -1));
		if(selected)
			selectedResource = data.MainHandle;

		if(focus)
			ImGui::PopStyleColor(3);

		// Drag and drop
		if(!data.IsDirectory)
		{
			if(data.MainHandle.IsValid() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptBeforeDelivery))
			{
				if(data.MainHandle.Type == D_SCENE::PrefabResource::GetResourceType())
				{
					D_SCENE::PrefabResourceDragDropPayloadContent payload;
					payload.Handle = data.MainHandle;
					payload.Type = std::to_string(data.MainHandle.Type);
					ImGui::SetDragDropPayload(D_PAYLOAD_TYPE_RESOURCE, &payload, sizeof(D_SCENE::PrefabResourceDragDropPayloadContent), ImGuiCond_Once);
				}
				else
				{
					D_RESOURCE::ResourceDragDropPayloadContent payload;
					payload.Handle = data.MainHandle;
					payload.Type = std::to_string(data.MainHandle.Type);

					ImGui::SetDragDropPayload(D_PAYLOAD_TYPE_RESOURCE, &payload, sizeof(D_RESOURCE::ResourceDragDropPayloadContent), ImGuiCond_Once);
				}
				ImGui::Text((data.Name + " (Resource)").c_str());
				ImGui::EndDragDropSource();
			}

		}

		doubleClicked = ImGui::IsMouseDoubleClicked(0) && ImGui::IsItemHovered();

		if(ImGui::IsItemHovered())
			ImGui::SetTooltip(data.Path.filename().string().c_str());

		ImGui::SetCursorPos(ImVec2((availWidth - size.x) / 2 + startCurPos.x, startCurPos.y + 5));

		ImGui::Image((ImTextureID)data.IconId, size);

		auto nameStr = data.Name.c_str();

		auto textWidth = ImGui::CalcTextSize(nameStr, 0, 0, availWidth).x;

		auto textStart = (availWidth - textWidth) / 2;
		textStart = textStart > 0 ? textStart : 0;

		ImGui::SetCursorPos(ImVec2(startCurPos.x + textStart, startCurPos.y + height - 1.5f * padding));

		ImGui::TextWrapped(nameStr);

		if(data.ChildResources.size() > 0)
		{
			// Showing child elements
			ImGui::SetCursorPos(ImVec2((availWidth - size.x) / 2 + startCurPos.x + size.x, startCurPos.y + 5));

			if(ImGui::Button(ICON_FA_CHEVRON_DOWN))
				ImGui::OpenPopup("ChildResourcesInContentWindowComponent");

			if(ImGui::BeginPopup("ChildResourcesInContentWindowComponent"))
			{
				static ImGuiListClipper clipper;
				clipper.Begin((int)data.ChildResources.size());
				while(clipper.Step())
				{
					for(int idx = clipper.DisplayStart; idx < clipper.DisplayEnd; idx++)
					{
						auto const& handle = data.ChildResources[idx];
						auto prev = D_RESOURCE::GetResourcePreview(handle);
						auto name = WSTR2STR(prev.Name);
						if(ImGui::Selectable((name + " (" + D_RESOURCE::Resource::GetResourceName(handle.Type).string() + ")").c_str()))
						{
							selected = true;
							selectedResource = handle;
						}

						// Drag and drop source
						if(handle.IsValid() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptBeforeDelivery))
						{

							if(handle.Type == D_SCENE::PrefabResource::GetResourceType())
							{
								D_SCENE::PrefabResourceDragDropPayloadContent payload;
								payload.Handle = handle;
								payload.Type = std::to_string(handle.Type);
								ImGui::SetDragDropPayload(D_PAYLOAD_TYPE_RESOURCE, &payload, sizeof(D_SCENE::PrefabResourceDragDropPayloadContent), ImGuiCond_Once);

							}
							else
							{
								D_RESOURCE::ResourceDragDropPayloadContent payload;
								payload.Handle = handle;
								payload.Type = std::to_string(handle.Type);

								ImGui::SetDragDropPayload(D_PAYLOAD_TYPE_RESOURCE, &payload, sizeof(D_RESOURCE::ResourceDragDropPayloadContent), ImGuiCond_Once);
							}
							ImGui::Text((name + " (Resource)").c_str());
							ImGui::EndDragDropSource();
						}

					}
				}
				ImGui::EndPopup();
			}
		}


		ImGui::EndChildFrame();

		ImGui::PopID();
	}
}
