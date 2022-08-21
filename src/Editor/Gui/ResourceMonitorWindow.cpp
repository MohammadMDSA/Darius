#include "Editor/pch.hpp"
#include "ResourceMonitorWindow.hpp"

#include <ResourceManager/ResourceManager.hpp>

#include <imgui/imgui.h>

using namespace D_RESOURCE;
using namespace D_CORE;

namespace Darius::Editor::Gui::Windows
{
	ResourceMonitorWindow::ResourceMonitorWindow()
	{
	}

	ResourceMonitorWindow::~ResourceMonitorWindow()
	{
	}

	void ResourceMonitorWindow::DrawGUI()
	{

		DVector<Resource*> resources;
		D_RESOURCE::GetAllResources(resources);

		auto flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_ScrollY;

		if (ImGui::BeginTable("Resource Table", 10, flags))
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

			ImGui::TableHeadersRow();

			ImGuiListClipper clipper;
			clipper.Begin(200);

			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < (int)resources.size() && row < clipper.DisplayEnd; row++)
				{
					auto resource = resources[row];
					auto resourceName = resource->GetName();
					auto resNameStr = STR_WSTR(resourceName);
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text(std::to_string(resource->GetId()).c_str());
					ImGui::TableSetColumnIndex(1);
					ImGui::Text(resNameStr.c_str());
					ImGui::TableSetColumnIndex(2);
					ImGui::Text(ResourceTypeToString(resource->GetType()).c_str());
					ImGui::TableSetColumnIndex(3);
					ImGui::Text(ToString(resource->GetUuid()).c_str());
					ImGui::TableSetColumnIndex(4);
					ImGui::Text(std::to_string(resource->GetDefault()).c_str());
					ImGui::TableSetColumnIndex(5);
					ImGui::Text(std::to_string(resource->GetLoaded()).c_str());
					ImGui::TableSetColumnIndex(6);
					ImGui::Text(std::to_string(resource->GetVersion()).c_str());
					ImGui::TableSetColumnIndex(7);
					ImGui::Text(std::to_string(resource->GetDirtyDisk()).c_str());
					ImGui::TableSetColumnIndex(8);
					ImGui::Text(std::to_string(resource->GetDirtyGPU()).c_str());
					ImGui::TableSetColumnIndex(9);
					ImGui::Text(resource->GetDefault() ? " - " : resource->GetPath().string().c_str());

				}
			}

			ImGui::EndTable();
		}
	}

}