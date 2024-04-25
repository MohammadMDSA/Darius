#include "Editor/pch.hpp"
#include "SceneGraphWindow.hpp"

#include "Editor/EditorContext.hpp"
#include "Editor/Simulation.hpp"
#include "Editor/GUI/GuiManager.hpp"
#include "Editor/GUI/Components/TreeClipper.hpp"

#include <Core/Input.hpp>
#include <Core/Containers/List.hpp>
#include <Engine/EngineContext.hpp>
#include <FBX/FBXPrefabResource.hpp>
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
		DVector<GameObject*> children;
		root.children([&](D_ECS::Entity e)
			{
				children.push_back(D_WORLD::GetGameObject(e));
			});

		std::sort(children.begin(), children.end(), [](GameObject* a, GameObject* b)
			{
				return (std::strcmp(a->GetName().c_str(), b->GetName().c_str()) < 0);
			});


		bool abort = false;
		static ImGuiListClipper clipper;
		clipper.Begin((int)children.size());
		while (clipper.Step())
		{
			for (int idx = clipper.DisplayStart; idx < D_MATH::Min((int)children.size(), clipper.DisplayEnd); idx++)
			{

				if (!abort)
					DrawObject(children[idx], selectedObj, abort);
			}
		}
		clipper.End();

		if (!ImGui::IsAnyItemHovered() && mHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
		}

	}

	void SceneGraphWindow::DrawObject(GameObject* go, GameObject* selectedObj, bool& abort)
	{
		ImGuiTreeNodeFlags baseFlag = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		auto childrenCount = go->CountChildren();
		if (!childrenCount)
			baseFlag |= ImGuiTreeNodeFlags_Leaf;

		auto selected = go == selectedObj;
		if (selected)
			baseFlag |= ImGuiTreeNodeFlags_Selected;

		auto objName = (go->GetPrefab().is_nil() ? ICON_FA_OBJECT_UNGROUP : ICON_FA_BOX) + std::string(std::string(" ") + std::string(go->GetName()));
		auto nodeOpen = ImGui::TreeNodeEx((void*)(go), baseFlag, objName.c_str());

		if (ImGui::BeginPopupContextItem())
		{
			ImGui::Text("Game Object");
			ImGui::Separator();

			D_GUI_MANAGER::DrawGammAddMenu(go);

			{
				auto selectedGo = D_EDITOR_CONTEXT::GetSelectedGameObject();
				bool valid = selectedGo && selectedGo->IsValid() && selectedGo->CanAttachTo(go);
				if(!valid)
					ImGui::BeginDisabled();
				if(ImGui::MenuItem(ICON_FA_ARROW_TURN_UP "  Set Parent"))
				{
					selectedGo->SetParent(go, GameObject::AttachmentType::KeepWorld);
					abort = true;
				}
				if(!valid)
					ImGui::EndDisabled();
			}

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
						pastedGo->SetParent(go, GameObject::AttachmentType::KeepLocal);
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
			payload.GameObjectRef = go;
			ImGui::SetDragDropPayload(D_PAYLOAD_TYPE_GAMEOBJECT, &payload, sizeof(D_SCENE::GameObjectDragDropPayloadContent), ImGuiCond_Once);
			ImGui::Text((std::string(go->GetName()) + std::string(" (Game Object)")).c_str());
			ImGui::EndDragDropSource();
		}

		// Handle the addition of child by dragging
		if (ImGui::BeginDragDropTarget())
		{
			ImGuiPayload const* imPayload = ImGui::GetDragDropPayload();
			auto payload = reinterpret_cast<D_UTILS::BaseDragDropPayloadContent const*>(imPayload->Data);

			if (payload && payload->PayloadType != D_UTILS::BaseDragDropPayloadContent::Type::Invalid)
			{

				if (auto goPayload = dynamic_cast<D_SCENE::GameObjectDragDropPayloadContent const*>(payload))
				{
					if (goPayload->GameObjectRef->CanAttachTo(go) && goPayload->GameObjectRef->IsValid() && ImGui::AcceptDragDropPayload(D_PAYLOAD_TYPE_GAMEOBJECT))
					{

						goPayload->GameObjectRef->SetParent(go, GameObject::AttachmentType::KeepWorld);
						abort = true;
					}
				}

				// In case it is a prefab resource
				else if (payload->IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type::Resource, std::to_string(D_SCENE::PrefabResource::GetResourceType())))
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
				else if (payload->IsCompatible(D_UTILS::BaseDragDropPayloadContent::Type::Resource, std::to_string(D_FBX::FBXPrefabResource::GetResourceType())))
				{
					auto payloadData = reinterpret_cast<D_RESOURCE::ResourceDragDropPayloadContent const*>(imPayload->Data);

					auto prefabResource = D_RESOURCE::GetResourceSync<D_FBX::FBXPrefabResource>(payloadData->Handle, true);

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
			if (!abort)
			{
				auto children = DVector<GameObject*>();
				children.reserve(go->CountChildren());
				go->VisitChildren([&](GameObject* child)
					{
						children.push_back(child);
					});
				std::sort(children.begin(), children.end(), [](GameObject* a, GameObject* b)
					{
						return std::strcmp(a->GetName().c_str(), b->GetName().c_str()) < 0;
					});
				for (auto child : children)
				{
					if (!abort)
						DrawObject(child, selectedObj, abort);
				}
			}
			ImGui::TreePop();
		}
	}

}
