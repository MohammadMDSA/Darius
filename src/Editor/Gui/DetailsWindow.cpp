#include "Editor/pch.hpp"
#include "DetailsWindow.hpp"
#include "Editor/EditorContext.hpp"

namespace Darius::Editor::Gui::Windows
{

	DetailsWindow::DetailsWindow()
	{
	}

	DetailsWindow::~DetailsWindow()
	{
	}

	void DetailsWindow::DrawGUI()
	{
		auto obj = D_EDITOR_CONTEXT::GetSelectedGameObject();
		if (obj)
			obj->DrawInspector();
	}
}
