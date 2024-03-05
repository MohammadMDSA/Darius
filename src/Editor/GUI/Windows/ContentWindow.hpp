#pragma once

#include "Window.hpp"
#include "Editor/GUI/Components/ContentWindowComponents.hpp"

#include <Core/Containers/Vector.hpp>
#include <Core/Filesystem/Path.hpp>
#include <Core/Signal.hpp>

#include <atomic>
#include <mutex>

#include "ContentWindow.generated.hpp"

namespace Darius::ResourceManager
{
	struct ResourceHandle;
}

namespace Darius::Editor::Gui::Windows
{
	class DClass(Serialize) ContentWindow : public Window
	{
		GENERATED_BODY();
		D_CH_EDITOR_WINDOW_BODY(ContentWindow, "Content");

	public:
		// Inherited via Window

		INLINE virtual void			Update(float) override {}

		virtual void				DrawGUI() override;

		void						UpdateDirectoryItems();
		bool						TrySetCurrentPath(D_FILE::Path const& path);

	private:

		struct TreeFoldeEntry
		{
			D_FILE::Path	Path;
			bool			Openned = false;
			std::string		Name;
			D_CONTAINERS::DVector<TreeFoldeEntry*> Children;
		};

		void						DrawMainItems();
		void						DrawBreadcrumb();
		void						DrawFolderTreeItem(TreeFoldeEntry& entry);
		void						SelectEditorContentItem(D_GUI_COMPONENT::EditorContentWindowItem const* item, D_RESOURCE::ResourceHandle const& selectedHandle);

		D_CONTAINERS::DVector<D_GUI_COMPONENT::EditorContentWindowItem>	mCurrentDirectoryItems;
		D_CONTAINERS::DVector<D_FILE::Path> mBreadcrumbItems; // it's from child to parent
		D_CONTAINERS::DUnorderedMap<D_FILE::Path, TreeFoldeEntry> mTreeViewFolderMap;
		TreeFoldeEntry*				mSelectedTreeNode = nullptr;
		D_GUI_COMPONENT::EditorContentWindowItem const* mSelectedItem = nullptr;

		DField(Get[const, &, inline])
		D_FILE::Path				mCurrentDirectory;
		float						mTreeViewWidth;
		float						mRightPanelWidth;

		std::atomic_uint			mNumberOfItemsToBeLoaded;
		std::mutex					mItemsLoadMutex;

		D_CORE::SignalConnection	mComponentChangePathConnection;
		D_CORE::SignalConnection	mResourceChangePathConnection;

		Darius::ResourceManager::ResourceHandle mFocusedResource;

	};
}

File_ContentWindow_GENERATED