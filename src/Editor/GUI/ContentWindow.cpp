#include "Editor/pch.hpp"
#include "ContentWindow.hpp"

#include "Editor/EditorContext.hpp"
#include "Components/ContentWindowComponents.hpp"

#include <Core/Filesystem/FileUtils.hpp>

#include <imgui.h>

namespace Darius::Editor::Gui::Windows
{
	ContentWindow::ContentWindow()
	{
		SetCurrentPath(D_EDITOR_CONTEXT::GetAssetsPath());
	}

	ContentWindow::~ContentWindow()
	{
	}

	void ContentWindow::DrawGUI()
	{

		ImGuiStyle& style = ImGui::GetStyle();
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		float buttonWidth = 100;

		for (auto const& dirItem : mCurrentDirectoryItems)
		{
			D_GUI_COMPONENT::EditorContentWindowItem itemData = { dirItem.second, mCurrentDirectory / dirItem.second, dirItem.first };
			bool selected = false;
			bool clicked;
			D_GUI_COMPONENT::ContentWindowItemGrid(itemData, buttonWidth, buttonWidth, selected, clicked);

			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + buttonWidth; // Expected position if next button was on same line
			if (next_button_x2 < window_visible_x2)
				ImGui::SameLine();
		}

	}

	void ContentWindow::UpdateDirectoryItems()
	{
		mCurrentDirectoryItems.clear();

		D_FILE::VisitEntriesInDirectory(mCurrentDirectory, false, [&](auto const& _path, bool isDir)
			{
				mCurrentDirectoryItems.push_back({ isDir, _path.filename().string() });
			});
	}

	void ContentWindow::SetCurrentPath(D_FILE::Path const& path)
	{
		mCurrentDirectory = path;
		UpdateDirectoryItems();
	}
}
