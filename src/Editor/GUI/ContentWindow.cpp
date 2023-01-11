#include "Editor/pch.hpp"
#include "ContentWindow.hpp"

#include "Editor/EditorContext.hpp"

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

		for (auto& dirItem : mCurrentDirectoryItems)
		{
			bool selected = false;
			bool clicked;
			D_GUI_COMPONENT::ContentWindowItemGrid(dirItem, buttonWidth, buttonWidth, selected, clicked);

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
				if (isDir)
				{
					mCurrentDirectoryItems.push_back({ _path.filename().string(), _path, true });
				}
				else
				{

					auto name = D_FILE::GetFileName(_path.filename());
					auto nameStr = STR_WSTR(name);
					mCurrentDirectoryItems.push_back({ nameStr, _path, false });

				}
			});

		DVector<Path> resVec;
		D_RESOURCE::GetAllResourcePaths(resVec);

		for (auto const& path : resVec)
		{
			if (path.parent_path().compare(mCurrentDirectory) == 0)
			{
				auto name = D_FILE::GetFileName(path.filename());
				auto nameStr = STR_WSTR(name);
				mCurrentDirectoryItems.push_back({ nameStr, path, false });
			}
		}

		std::sort(mCurrentDirectoryItems.begin(), mCurrentDirectoryItems.end(), [](auto first, auto second)
			{
				if (first.IsDirectory && !second.IsDirectory)
					return true;
				else if (second.IsDirectory && !first.IsDirectory)
					return false;
				else
					return first.Name.compare(second.Name.c_str()) < 0;
			});
	}

	void ContentWindow::SetCurrentPath(D_FILE::Path const& path)
	{
		mCurrentDirectory = path;
		UpdateDirectoryItems();
	}
}
