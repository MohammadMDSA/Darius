#include "Editor/pch.hpp"
#include "SceneGraphWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <Scene/Scene.hpp>

#include <imgui/imgui.h>

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

		DrawObjList(gos, selectedObj);
	}

	void SceneGraphWindow::DrawObjList(DVector<GameObject*> const& gos, GameObject* selectedObj)
	{
		ImGuiTreeNodeFlags baseFlag = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Leaf;

		auto idx = 0;
		for (auto& go : gos)
		{
			ImGuiTreeNodeFlags nodeFlag = baseFlag;

			auto selected = go == selectedObj;
			if (selected)
				nodeFlag |= ImGuiTreeNodeFlags_Selected;

			auto nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)(idx++), nodeFlag, go->GetName().c_str());

			if (ImGui::IsItemClicked() && !selected)
				D_EDITOR_CONTEXT::SetSelectedGameObject(go);

			if (nodeOpen)
				ImGui::TreePop();
		}
	}

}
