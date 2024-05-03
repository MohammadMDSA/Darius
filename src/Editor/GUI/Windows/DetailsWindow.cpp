#include "Editor/pch.hpp"
#include "DetailsWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include <imgui.h>

#include <ResourceManager/ResourceLoader.hpp>

using namespace D_RESOURCE;

namespace Darius::Editor::Gui::Windows
{

	DetailsWindow::DetailsWindow(D_SERIALIZATION::Json& config) :
		Window(config)
	{
	}

	DetailsWindow::~DetailsWindow()
	{
	}

	void DetailsWindow::DrawGUI()
	{
		D_PROFILING::ScopedTimer profiling(L"Details Window Draw GUI");
		auto obj = D_EDITOR_CONTEXT::GetSelectedDetailed();
		if (!obj)
			return;

		auto editable = obj->IsEditableInDetailsWindow();

		

		if (auto resource = dynamic_cast<Resource*>(obj); resource)
		{
			if (!resource->IsLoaded())
			{
				//D_RESOURCE_LOADER::LoadResourceAsync(resource, nullptr);

				return;
			}
		}

		// Disable inputs if the resource is a default one
		if (!editable)
			ImGui::BeginDisabled(true);
		
		ImGui::Text("Name: ");
		ImGui::SameLine();
		ImGui::Text(obj->GetDetailedName().c_str());

		// Parent
		{
			auto parent = obj->GetDetailedParent();
			ImGui::Text("Parent:");
			ImGui::SameLine();
			if(!parent)
				ImGui::Button("<None>");
			else
			{
				if(ImGui::Button(parent->GetDetailedName().c_str()))
					D_EDITOR_CONTEXT::SetSelectedDetailed(parent);
			}
		}

		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		obj->DrawDetails(nullptr);

		if (!editable)
			ImGui::EndDisabled();
	}
}
