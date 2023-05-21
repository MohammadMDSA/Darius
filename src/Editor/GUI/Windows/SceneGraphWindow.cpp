#include "Editor/pch.hpp"
#include "SceneGraphWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <Core/Input.hpp>
#include <Core/Containers/List.hpp>
#include <Engine/EngineContext.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Resources/PrefabResource.hpp>
#include <Scene/Utils/GameObjectDragDropPayload.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <Libs/imgui_wrapper/ImGuiFileDialog/ImGuiFileDialog.h>

using namespace D_CONTAINERS;
using namespace D_ECS;
using namespace D_SCENE;

namespace Darius::Editor::Gui::Windows
{

	SceneGraphWindow::SceneGraphWindow(D_SERIALIZATION::Json const& config) :
		Window(config)
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

		auto objName = (go->GetPrefab() ? ICON_FA_BOX : ICON_FA_OBJECT_UNGROUP) + (" " + go->GetName());
		auto nodeOpen = ImGui::TreeNodeEx((void*)(go), baseFlag, objName.c_str());

		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text("Game Object");
			ImGui::Separator();

			if (ImGui::Selectable("Add Game Object"))
			{
				D_WORLD::CreateGameObject()->SetParent(go);
			}

			else if (ImGui::Selectable("Delete"))
			{
				D_WORLD::DeleteGameObjectImmediately(go);
				if (selected)
					D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
				ImGui::EndPopup();
				ImGui::TreePop();
				return;
			}

			if (!go->GetPrefab())
				if (ImGui::Selectable("Create Prefab"))
				{
					ImGuiFileDialog::Instance()->OpenDialog("SavePrefab", "Create Prefab", ".prefab", D_ENGINE_CONTEXT::GetAssetsPath().string(), 1, go);
				}


			ImGui::EndPopup();
		}

		// Handle Selecting it
		if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left) && !selected)
			D_EDITOR_CONTEXT::SetSelectedGameObject(go);

		// Handle Drag Drop Source
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_AcceptBeforeDelivery))
		{
			D_SCENE::GameObjectDragDropPayloadContent payload;
			payload.GameObject = go;
			ImGui::SetDragDropPayload(D_PAYLOAD_TYPE_GAMEOBJECT, &payload, sizeof(D_SCENE::GameObjectDragDropPayloadContent), ImGuiCond_Once);
			ImGui::Text((go->GetName() + " (Game Object)").c_str());
			ImGui::EndDragDropSource();
		}

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
