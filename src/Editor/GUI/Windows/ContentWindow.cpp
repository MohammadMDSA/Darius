#include "Editor/pch.hpp"
#include "ContentWindow.hpp"

#include "Editor/EditorContext.hpp"
#include "Editor/GUI/Components/Common.hpp"
#include "Editor/GUI/ThumbnailManager.hpp"

#include <Core/Filesystem/FileUtils.hpp>
#include <Engine/EngineContext.hpp>
#include <FBX/FBXPrefabResource.hpp>
#include <ResourceManager/Resource.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include <Libs/FontIcon/IconsFontAwesome6.h>

#include <imgui.h>

#include "ContentWindow.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_FILE;

namespace Darius::Editor::Gui::Windows
{
	ContentWindow::ContentWindow(D_SERIALIZATION::Json& config) :
		Window(config),
		mTreeViewWidth(-1.f),
		mRightPanelWidth(-1.f),
		mFocusScrollDone(false)
	{
		auto assetsPath = D_ENGINE_CONTEXT::GetAssetsPath();
		TrySetCurrentPath(assetsPath);

		mTreeViewFolderMap[assetsPath] = { assetsPath, false, assetsPath.parent_path().filename().string() };

		// Setup listeners
		mComponentChangePathConnection = D_ECS_COMP::ComponentBase::RequestPathChange.connect([&](D_FILE::Path const& path, D_RESOURCE::ResourceHandle const& handle, bool select)
			{
				if (select)
					D_EDITOR_CONTEXT::SetSelectedDetailed(D_RESOURCE::GetRawResourceSync(handle));

				ContentWindow::TrySetCurrentPath(path);
				mFocusedResource = handle;
				mFocusScrollDone = false;
			});

		mResourceChangePathConnection = D_RESOURCE::Resource::RequestPathChange.connect([&](D_FILE::Path const& path, D_RESOURCE::ResourceHandle const& handle, bool select)
			{
				if (select)
					D_EDITOR_CONTEXT::SetSelectedDetailed(D_RESOURCE::GetRawResourceSync(handle));

				ContentWindow::TrySetCurrentPath(path);
				mFocusedResource = handle;
				mFocusScrollDone = false;
			});

	}

	ContentWindow::~ContentWindow()
	{
		mComponentChangePathConnection.disconnect();
		mResourceChangePathConnection.disconnect();
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
			TrySetCurrentPath(entry.Path);
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
			TrySetCurrentPath(newCurrentDir);
	}

	void ContentWindow::DrawMainItems()
	{
		// Are items are being loaded
		if (mNumberOfItemsToBeLoaded.load() > 0)
		{
			ImGui::Text("Items are being loaded");
			return;
		}

		ImGuiStyle& style = ImGui::GetStyle();
		float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;

		float buttonWidth = 100;

		Path newContext;

		for (auto& dirItem : mCurrentDirectoryItems)
		{
			bool selected = mSelectedItem == &dirItem;
			D_RESOURCE::ResourceHandle selectedHandle = D_RESOURCE::EmptyResourceHandle;
			bool clicked;

			bool focused = false;
			if (mFocusedResource.IsValid())
			{
				for (auto const& subRes : dirItem.ChildResources)
					if (mFocusedResource == subRes)
					{
						focused = true;
						break;
					}
			}

			D_GUI_COMPONENT::ContentWindowItemGrid(dirItem, buttonWidth, buttonWidth, focused, selected, clicked, selectedHandle);

			if (focused && !mFocusScrollDone)
			{
				ImGui::SetScrollHereY();
				mFocusScrollDone = true;
			}

			if (selected)
				mFocusedResource = D_RESOURCE::EmptyResourceHandle;

			// Check if becomes selected
			if (selected && mSelectedItem != &dirItem)
				SelectEditorContentItem(&dirItem, selectedHandle);

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
			TrySetCurrentPath(newContext);

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
		D_RESOURCE::GetAllResourcePathsInDirectory(mCurrentDirectory, resVec);

		mNumberOfItemsToBeLoaded.store(0);

		D_THUMBNAIL::RegisterExistingResources(mCurrentDirectory);

		for (auto const& path : resVec)
		{

			mNumberOfItemsToBeLoaded++;
			D_RESOURCE::ResourceLoader::LoadResourceAsync(path, [mutex = &mItemsLoadMutex, _currentDirectoryItems = &mCurrentDirectoryItems, path = path, itemsLoading = &mNumberOfItemsToBeLoaded](auto containedResources)
				{
					std::lock_guard lock(*mutex);
					auto& currentDirectoryItems = *_currentDirectoryItems;
					auto name = D_FILE::GetFileName(path.filename());
					auto nameStr = WSTR2STR(name);
					(*itemsLoading)--;

					uint64_t icon = D_THUMBNAIL::GetIconTextureId(D_THUMBNAIL::CommonIcon::File);

					D_RESOURCE::ResourceHandle resourceHandle = D_RESOURCE::EmptyResourceHandle;
					if (containedResources.size() == 1)
					{
						icon = D_THUMBNAIL::GetResourceTextureId(containedResources[0]);
						resourceHandle = containedResources[0];
					}

					currentDirectoryItems.push_back({ nameStr, path, false, icon, resourceHandle });
					auto& lastItem = currentDirectoryItems[currentDirectoryItems.size() - 1];

					for (auto const& handle : containedResources)
					{
						if (handle.Type == D_FBX::FBXPrefabResource::GetResourceType())
						{
							lastItem.MainHandle = handle;
							lastItem.IconId = D_THUMBNAIL::GetResourceTextureId(handle);
						}
						else
						{
							lastItem.ChildResources.push_back(handle);
						}
					}

					if (itemsLoading->load() != 0)
						return;

					std::sort(currentDirectoryItems.begin(), currentDirectoryItems.end(), [](auto first, auto second)
						{
							if (first.IsDirectory && !second.IsDirectory)
								return true;
							else if (second.IsDirectory && !first.IsDirectory)
								return false;
							else
								return first.Name.compare(second.Name.c_str()) < 0;
						});
				}, true);

		}
	}

	bool ContentWindow::TrySetCurrentPath(D_FILE::Path const& path)
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

	void ContentWindow::SelectEditorContentItem(D_GUI_COMPONENT::EditorContentWindowItem const* item, D_RESOURCE::ResourceHandle const& selectedHandle)
	{
		mSelectedItem = item;

		if (item->IsDirectory)
			return;

		auto containedResources = D_RESOURCE_LOADER::LoadResourceSync(item->Path);
		if (containedResources.size() == 0)
		{
			D_EDITOR_CONTEXT::SetSelectedDetailed(nullptr);
			return;
		}

		auto resource = D_RESOURCE::GetRawResourceSync(selectedHandle, true);
		D_EDITOR_CONTEXT::SetSelectedDetailed(resource);

		/*D_RESOURCE::ResourceLoader::LoadResourceAsync(item->Path, [selectedHandle](auto containedResources)
			{
				if (containedResources.size() == 0)
				{
					D_EDITOR_CONTEXT::SetSelectedDetailed(nullptr);
					return;
				}

				auto resource = D_RESOURCE::GetRawResourceSync(selectedHandle);
				D_EDITOR_CONTEXT::SetSelectedDetailed(resource);
			});*/
	}

}
