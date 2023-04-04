#pragma once

#include "Window.hpp"
#include "Editor/GUI/Components/ContentWindowComponents.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Vector.hpp>

namespace Darius::Editor::Gui::Windows
{
	class ContentWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY(ContentWindow, "Content");

	public:
		// Inherited via Window

		INLINE virtual void			Update(float) override {}

		virtual void				DrawGUI() override;

		void						UpdateDirectoryItems();
		bool						SetCurrentPath(D_FILE::Path const& path);

		D_CH_R_FIELD(D_FILE::Path, CurrentDirectory);

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
		void						SelectEditorContentItem(D_GUI_COMPONENT::EditorContentWindowItem const* item);

		D_CONTAINERS::DVector<D_GUI_COMPONENT::EditorContentWindowItem>	mCurrentDirectoryItems;
		D_CONTAINERS::DVector<D_FILE::Path> mBreadcrumbItems; // it's from child to parent
		D_CONTAINERS::DUnorderedMap<D_FILE::Path, TreeFoldeEntry> mTreeViewFolderMap;
		TreeFoldeEntry*				mSelectedTreeNode = nullptr;
		D_GUI_COMPONENT::EditorContentWindowItem const* mSelectedItem = nullptr;

		float						mTreeViewWidth;
		float						mRightPanelWidth;
	};
}
