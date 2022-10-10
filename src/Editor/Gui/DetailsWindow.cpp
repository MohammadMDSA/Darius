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
		D_PROFILING::ScopedTimer profiling(L"Details Window Draw GUI");
		auto obj = D_EDITOR_CONTEXT::GetSelectedDetailed();
		if (!obj)
			return;

		obj->DrawDetails(nullptr);
	}
}
