#include "Editor/pch.hpp"
#include "ContentWindow.hpp"

#include "Editor/EditorContext.hpp"
#include "Editor/GUI/Components/Common.hpp"
#include "Editor/GUI/ThumbnailManager.hpp"

#include <Core/Filesystem/FileUtils.hpp>
#include <Engine/EngineContext.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

#include "ContentWindow.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_FILE;

namespace Darius::Editor::Gui::Windows
{
	ContentWindow::ContentWindow(D_SERIALIZATION::Json const& config) :
		Window(config),
		mTreeViewWidth(-1.f),
		mRightPanelWidth(-1.f)
	{
		auto assetsPath = D_ENGINE_CONTEXT::GetAssetsPath();
		SetCurrentPath(assetsPath);

		mTreeViewFolderMap[assetsPath] = { assetsPath, false, assetsPath.parent_path().filename().string() };
	}

	ContentWindow::~ContentWindow()
	{
	}

	void ContentWindow::DrawGUI()
	{
		auto availableWidth = ImGui::GetContentRegionAvail().x;
		auto availableHeigh = ImGui::GetContentRegionAvail().y;

		if (mTreeViewWidth < 0)
			mTreeViewWidth = availableWidth / 5;

		mRightPanelWidth = availableWidth - mTreeViewWidth - 8.f;

		D_GUI_COMPONENT::Splitter(true, 8.f, &mTreeViewWidth, &mRightPanelWidth, 50.f, 50.f, availableHeigh);

		ImGui::BeginChild("##FileTreeView", ImVec2(mTreeViewWidth, availableHeigh));
		{
			DrawFolderTreeItem(mTreeViewFolderMap[D_ENGINE_CONTEXT::GetAssetsPath()]);
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("##MainPane", ImVec2(mRightPanelWidth, availableHeigh));
		{
			ImGui::BeginChild("##ToolbarPane", ImVec2(mRightPanelWidth, 40));
			{
				DrawBreadcrumb();
			}
			ImGui::EndChild();

			ImGui::BeginChild("##ItemsPane");
			{
				DrawMainItems();
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();
	}

	void ContentWindow::DrawFolderTreeItem(TreeFoldeEntry& entry)
	{
		ImGuiTreeNodeFlags baseFlag = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;

		if (entry.Openned && entry.Children.size() == 0)
			baseFlag |= ImGuiTreeNodeFlags_Leaf;

		if (&entry == mSelectedTreeNode)
			baseFlag |= ImGuiTreeNodeFlags_Selected;

		auto nodeOpen = ImGui::TreeNodeEx((void*)(&entry), baseFlag, entry.Name.c_str());

		if (ImGui::IsItemClicked())
		{
			mSelectedTreeNode = &entry;
			SetCurrentPath(entry.Path);
		}

		if (nodeOpen)
		{
			// Creating records for children
			if (!entry.Openned)
			{
				entry.Openned = true;
				D_FILE::VisitEntriesInDirectory(entry.Path, false, [&](Path const& _path, bool isDir)
					{
						if (!isDir)
							return;

						mTreeViewFolderMap[_path] = { _path, false, _path.filename().string(), D_CONTAINERS::DVector<TreeFoldeEntry*>() };

						entry.Children.push_back(&mTreeViewFolderMap[_path]);
					});

				std::sort(entry.Children.begin(), entry.Children.end(), [](auto const& a, auto const& b)
					{
						return b->Name > a->Name;
					});
			}

			// Drawing children
			for (auto child : entry.Children)
			{
				DrawFolderTreeItem(*child);
			}

			ImGui::TreePop();
		}

	}

	void ContentWindow::DrawBreadcrumb()
	{
		ImGui::SameLine(10.f);

		Path newCurrentDir;

		for (int i = (int)mBreadcrumbItems.size() - 1; i >= 0; i--)
		{
			if (ImGui::Button(mBreadcrumbItems[i].filename().string().c_str()))
				newCurrentDir = mBreadcrumbItems[i];

			if (i > 0)
			{
				ImGui::SameLine();
				ImGui::Text(ICON_FA_ARROW_RIGHT);
				ImGui::SameLine();
			}
		}
		ImGui::Separator();

		if (!newCurrentDir.empty())
			SetCurrentPath(newCurrentDir);
	}

	void ContentWindow::DrawMainItems()
	{

		ImGuiStyle& style = ImGui::GetStyle();
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		float buttonWidth = 100;

		Path newContext;

		for (auto& dirItem : mCurrentDirectoryItems)
		{
			bool selected = mSelectedItem == &dirItem;
			bool clicked;
			D_GUI_COMPONENT::ContentWindowItemGrid(dirItem, buttonWidth, buttonWidth, selected, clicked);

			// Check if becomes selected
			if (selected && mSelectedItem != &dirItem)
				SelectEditorContentItem(&dirItem);

			if (clicked && dirItem.IsDirectory)
				newContext = dirItem.Path;

			// Horzonal layout stuff
			float last_button_x2 = ImGui::GetItemRectMax().x;
			float next_button_x2 = last_button_x2 + style.ItemSpacing.x + buttonWidth; // Expected position if next button was on same line
			if (next_button_x2 < window_visible_x2)
				ImGui::SameLine();
		}

		// Switching to new director
		if (!newContext.empty())
			SetCurrentPath(newContext);

	}

	void ContentWindow::UpdateDirectoryItems()
	{
		mCurrentDirectoryItems.clear();

		D_FILE::VisitEntriesInDirectory(mCurrentDirectory, false, [&](auto const& _path, bool isDir)
			{
				if (isDir)
				{
					mCurrentDirectoryItems.push_back({ _path.filename().string(), _path, true, D_THUMBNAIL::GetIconTextureId(D_THUMBNAIL::CommonIcon::Folder) });
				}
			});

		DVector<Path> resVec;
		D_RESOURCE::GetAllResourcePaths(resVec);


		for (auto const& path : resVec)
		{
			auto parent = path.parent_path();
			if (parent.empty())
				continue;
			if (std::filesystem::equivalent(mCurrentDirectory, parent))
			{
				auto containedResources = D_RESOURCE::ResourceLoader::LoadResource(path, true);
				auto name = D_FILE::GetFileName(path.filename());
				auto nameStr = WSTR2STR(name);

				uint64_t icon = D_THUMBNAIL::GetIconTextureId(D_THUMBNAIL::CommonIcon::File);

				D_RESOURCE::ResourceHandle resourceHandle = D_RESOURCE::EmptyResourceHandle;
				if (containedResources.size() == 1)
				{
					icon = D_THUMBNAIL::GetResourceTextureId(containedResources[0]);
					resourceHandle = containedResources[0];
				}

				mCurrentDirectoryItems.push_back({ nameStr, path, false, icon, resourceHandle });
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

	bool ContentWindow::SetCurrentPath(D_FILE::Path const& path)
	{

		if (!D_H_ENSURE_DIR(path))
			return false;

		// Checking whether it is a child of assets folder
		auto assetsPath = D_ENGINE_CONTEXT::GetAssetsPath();

		auto newPath = path;
		auto temp = newPath;
		bool valid = false;

		while (!temp.empty())
		{
			if (std::filesystem::equivalent(temp, assetsPath))
			{
				valid = true;
				break;
			}
			temp = temp.parent_path();
		}
		if (!valid)
			return false;

		// Create breadcrumb items
		mBreadcrumbItems.clear();
		temp = newPath;
		while (!std::filesystem::equivalent(assetsPath, temp))
		{
			mBreadcrumbItems.push_back(temp);
			temp = temp.parent_path();
		}
		mBreadcrumbItems.push_back(assetsPath.parent_path());

		mCurrentDirectory = newPath.lexically_normal();
		UpdateDirectoryItems();
		return true;
	}

	void ContentWindow::SelectEditorContentItem(D_GUI_COMPONENT::EditorContentWindowItem const* item)
	{
		mSelectedItem = item;

		if (item->IsDirectory)
			return;

		auto containedResources = D_RESOURCE::ResourceLoader::LoadResource(item->Path, true);

		if (containedResources.size() == 0)
		{
			D_EDITOR_CONTEXT::SetSelectedDetailed(nullptr);
			return;
		}

		auto resource = D_RESOURCE::_GetRawResource(containedResources[0]);
		D_EDITOR_CONTEXT::SetSelectedDetailed(resource);
	}

}
