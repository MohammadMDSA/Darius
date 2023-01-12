#include "Editor/pch.hpp"
#include "SceneGraphWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <Core/Input.hpp>
#include <Core/Containers/List.hpp>
#include <Scene/Scene.hpp>

#include <imgui/imgui.h>

using namespace D_ECS;
using namespace D_CONTAINERS;

namespace Darius::Editor::Gui::Windows
{

	SceneGraphWindow::SceneGraphWindow()
	{
	}

	void SceneGraphWindow::Render(D_GRAPHICS::GraphicsContext&)
	{
	}

	void SceneGraphWindow::Update(float)
	{
	}

	void SceneGraphWindow::DrawGUI()
	{
		D_PROFILING::ScopedTimer profiling(L"Scene Graph Window Draw GUI");
		
		D_CONTAINERS::DVector<GameObject*> gos;
		D_WORLD::GetGameObjects(gos);
		auto selectedObj = D_EDITOR_CONTEXT::GetSelectedGameObject();

		// Iterating over root game objects
		auto root = D_WORLD::GetRoot();
		DList<GameObject*> children;
		root.children([&](D_ECS::Entity e)
			{
				children.push_back(D_WORLD::GetGameObject(e));
			});
		for (auto it = children.begin(); it != children.end(); it++)
		{
			DrawObject(*it, selectedObj);
		}

		if (!ImGui::IsAnyItemHovered() && mHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
		}
	}

	void SceneGraphWindow::DrawObject(GameObject* go, GameObject* selectedObj)
	{
		ImGuiTreeNodeFlags baseFlag = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		auto childrenCount = go->CountChildren();
		if (!childrenCount)
			baseFlag |= ImGuiTreeNodeFlags_Leaf;

		auto selected = go == selectedObj;
		if (selected)
			baseFlag |= ImGuiTreeNodeFlags_Selected;

		auto nodeOpen = ImGui::TreeNodeEx((void*)(go), baseFlag, go->GetName().c_str());

		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text("Game Object");
			ImGui::Separator();

			if (ImGui::Selectable("Add Game Object"))
			{
				D_WORLD::CreateGameObject()->SetParent(go);
			}

			if (ImGui::Selectable("Delete"))
			{
				D_WORLD::DeleteGameObject(go);
				if (selected)
					D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
			}

			ImGui::EndPopup();
		}

		if (ImGui::IsItemClicked() && !selected)
			D_EDITOR_CONTEXT::SetSelectedGameObject(go);

		if (nodeOpen)
		{
			auto children = DList<GameObject*>();
			go->VisitChildren([&](GameObject* child)
				{
					children.push_back(child);
				});

			for (auto child : children)
			{
				DrawObject(child, selectedObj);
			}

			ImGui::TreePop();
		}
	}

}
