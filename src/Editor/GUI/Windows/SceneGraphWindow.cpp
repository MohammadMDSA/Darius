#include "Editor/pch.hpp"
#include "SceneGraphWindow.hpp"

#include "Editor/EditorContext.hpp"
#include "Editor/Simulation.hpp"
#include "Editor/GUI/GuiManager.hpp"

#include <Core/Input.hpp>
#include <Core/Containers/List.hpp>
#include <Engine/EngineContext.hpp>
#include <Renderer/Resources/FBXPrefabResource.hpp>
#include <ResourceManager/ResourceDragDropPayload.hpp>
#include <ResourceManager/ResourceManager.hpp>
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

	SceneGraphWindow::SceneGraphWindow(D_SERIALIZATION::Json& config) :
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

		auto objName = (go->GetPrefab().is_nil() ? ICON_FA_OBJECT_UNGROUP : ICON_FA_BOX) + (" " + go->GetName());
		auto nodeOpen = ImGui::TreeNodeEx((void*)(go), baseFlag, objName.c_str());

		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text("Game Object");
			ImGui::Separator();

			D_GUI_MANAGER::DrawGammAddMenu(go);

			if (ImGui::MenuItem(ICON_FA_TRASH "  Delete"))
			{
				if (D_SIMULATE::IsSimulating())
					D_WORLD::DeleteGameObject(go);
				else
					D_WORLD::DeleteGameObjectImmediately(go);

				if (selected)
					D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
				ImGui::EndPopup();
				ImGui::TreePop();
				return;
			}
			else if (ImGui::MenuItem(ICON_FA_COPY "  Copy"))
			{
				D_EDITOR_CONTEXT::SetClipboard(go);
			}

			{
				bool pasteEnable = D_EDITOR_CONTEXT::IsGameObjectInClipboard();

				if (!pasteEnable)
					ImGui::BeginDisabled();
				if (ImGui::Selectable(ICON_FA_PASTE "  Paste"))
				{
					if (D_EDITOR_CONTEXT::IsGameObjectInClipboard())
					{
						GameObject* pastedGo;
						D_SERIALIZATION::Json goJson;
						D_EDITOR_CONTEXT::GetClipboardJson(true, goJson);
						D_WORLD::LoadGameObject(goJson, &pastedGo, true);
						pastedGo->SetParent(go, GameObject::AttachmentType::KeepWorld);
					}
				}
				if (!pasteEnable)
					ImGui::EndDisabled();
			}

			if (ImGui::Selectable(ICON_FA_WRENCH "  Create Prefab"))
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

		// Handle the addition of child by dragging
		if (ImGui::BeginDragDropTarget())
		{
			ImGuiPayload const* imPayload = ImGui::GetDragDropPayload();
			auto payload = reinterpret_cast<D_UTILS::BaseDragDropPayloadContent const*>(imPayload->Data);

			if (payload && payload->PayloadType != D_UTILS::BaseDragDropPayloadContent::Type::Invalid)
			{

				// In case it is a prefab resource
				if (payload->IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type::Resource, std::to_string(D_SCENE::PrefabResource::GetResourceType())))
				{
					auto payloadData = reinterpret_cast<D_RESOURCE::ResourceDragDropPayloadContent const*>(imPayload->Data);

					auto prefabResource = D_RESOURCE::GetResourceSync<PrefabResource>(payloadData->Handle, true);

					if (prefabResource.IsValid() && prefabResource->GetPrefabGameObject() && prefabResource->GetPrefabGameObject()->IsValid() && ImGui::AcceptDragDropPayload(D_PAYLOAD_TYPE_RESOURCE))
					{
						auto newGo = D_WORLD::InstantiateGameObject(prefabResource->GetPrefabGameObject(), true);
						newGo->SetParent(go, GameObject::AttachmentType::KeepLocal);
					}


				}

				// In case it is a fbx prefab resource
				else if (payload->IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type::Resource, std::to_string(D_RENDERER::FBXPrefabResource::GetResourceType())))
				{
					auto payloadData = reinterpret_cast<D_RESOURCE::ResourceDragDropPayloadContent const*>(imPayload->Data);

					auto prefabResource = D_RESOURCE::GetResourceSync<D_RENDERER::FBXPrefabResource>(payloadData->Handle, true);

					if (prefabResource.IsValid() && prefabResource->GetPrefabGameObject() && prefabResource->GetPrefabGameObject()->IsValid() && ImGui::AcceptDragDropPayload(D_PAYLOAD_TYPE_RESOURCE))
					{
						auto newGo = D_WORLD::InstantiateGameObject(prefabResource->GetPrefabGameObject(), true);
						newGo->SetParent(go, GameObject::AttachmentType::KeepLocal);
					}



				}
			}
			ImGui::EndDragDropTarget();
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
