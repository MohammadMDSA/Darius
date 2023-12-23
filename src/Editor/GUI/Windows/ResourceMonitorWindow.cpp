#include "Editor/pch.hpp"
#include "ResourceMonitorWindow.hpp"

#include "Editor/EditorContext.hpp"

#include <ResourceManager/ResourceManager.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include <imgui.h>

using namespace D_CONTAINERS;
using namespace D_CORE;
using namespace D_RESOURCE;

namespace Darius::Editor::Gui::Windows
{
	ResourceMonitorWindow::ResourceMonitorWindow(D_SERIALIZATION::Json& config) :
		Window(config)
	{
	}

	ResourceMonitorWindow::~ResourceMonitorWindow()
	{
	}

	void ResourceMonitorWindow::DrawGUI()
	{
		D_PROFILING::ScopedTimer profiling(L"Resource Window Draw GUI");

		DVector<Resource*> resources;
		D_RESOURCE::GetAllResources(resources);

		auto flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

		if (ImGui::BeginTable("Resource Table", 11, flags))
		{
			ImGui::TableSetupScrollFreeze(0, 1);
			ImGui::TableSetupColumn("Id");
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Type");
			ImGui::TableSetupColumn("UUID");
			ImGui::TableSetupColumn("Default");
			ImGui::TableSetupColumn("Loaded");
			ImGui::TableSetupColumn("Version");
			ImGui::TableSetupColumn("Disk Dirty");
			ImGui::TableSetupColumn("Gpu Dirty");
			ImGui::TableSetupColumn("Path");
			ImGui::TableSetupColumn("Owners");

			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin(200);

			static int selected;
			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < (int)resources.size() && row < clipper.DisplayEnd; row++)
				{
					auto resource = resources[row];
					auto resourceName = resource->GetName();
					auto resNameStr = WSTR2STR(resourceName);
					if (resource->IsDirtyDisk())
						resNameStr += "*";

					ImGui::TableNextRow();


					ImGui::TableSetColumnIndex(0);
					// Selectable row
					bool isSelected = row == selected;
					if (ImGui::Selectable(std::to_string(resource->GetId()).c_str(), &isSelected, ImGuiSelectableFlags_SpanAllColumns))
					{
						selected = row;
						D_EDITOR_CONTEXT::SetSelectedDetailed(resource);
					}

					ImGui::TableSetColumnIndex(1);
					ImGui::Text(resNameStr.c_str());
					ImGui::TableSetColumnIndex(2);
					ImGui::Text(ResourceTypeToString(resource->GetType()).c_str());
					ImGui::TableSetColumnIndex(3);
					ImGui::Text(ToString(resource->GetUuid()).c_str());
					ImGui::TableSetColumnIndex(4);
					ImGui::Text(std::to_string(resource->IsDefault()).c_str());
					ImGui::TableSetColumnIndex(5);
					ImGui::Text(std::to_string(resource->IsLoaded()).c_str());
					ImGui::TableSetColumnIndex(6);
					ImGui::Text(std::to_string(resource->GetVersion()).c_str());
					ImGui::TableSetColumnIndex(7);
					ImGui::Text(std::to_string(resource->IsDirtyDisk()).c_str());
					ImGui::TableSetColumnIndex(8);
					ImGui::Text(std::to_string(resource->IsDirtyGPU()).c_str());
					ImGui::TableSetColumnIndex(9);
					ImGui::Text(resource->IsDefault() ? " - " : resource->GetPath().string().c_str());
					ImGui::TableSetColumnIndex(10);
					ImGui::Text(std::to_string(resource->GetReferenceCount()).c_str());

				}
			}

			ImGui::EndTable();
		}
	}

}