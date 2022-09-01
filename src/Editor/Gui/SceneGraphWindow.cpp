#include "Editor/pch.hpp"
#include "SceneGraphWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <Core/Input.hpp>
#include <Scene/Scene.hpp>

#include <imgui/imgui.h>

using namespace D_ECS;

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
		D_CONTAINERS::DVector<GameObject*> gos;
		D_WORLD::GetGameObjects(gos);
		auto selectedObj = D_EDITOR_CONTEXT::GetSelectedGameObject();

		D_WORLD::GetRoot().children([&](D_ECS::Entity e)
			{
				DrawObject(D_WORLD::GetGameObject(e), selectedObj);
			});

		if (!ImGui::IsAnyItemHovered() && mHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
		}
	}

	void SceneGraphWindow::DrawObject(GameObject* go, GameObject* selectedObj)
	{
		ImGuiTreeNodeFlags baseFlag = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Leaf;

		ImGuiTreeNodeFlags nodeFlag = baseFlag;

		auto selected = go == selectedObj;
		if (selected)
			nodeFlag |= ImGuiTreeNodeFlags_Selected;

		auto nodeOpen = ImGui::TreeNodeEx((void*)(go), nodeFlag, go->GetName().c_str());
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::Selectable("Delete Game Object"))
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
			go->VisitChildren([&](GameObject* child)
				{
					DrawObject(child, selectedObj);
				});

			ImGui::TreePop();
		}
	}

}
