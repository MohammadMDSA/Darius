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
		ContentWindow(D_SERIALIZATION::Json const& config);
		~ContentWindow();

		// Inherited via Window

		INLINE virtual void			Render(D_GRAPHICS::GraphicsContext&) override {}

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
		D_CONTAINERS::DUnorderedMap<Path, TreeFoldeEntry> mTreeViewFolderMap;
		TreeFoldeEntry*				mSelectedTreeNode = nullptr;
		D_GUI_COMPONENT::EditorContentWindowItem const* mSelectedItem = nullptr;

		float						mTreeViewWidth;
		float						mRightPanelWidth;
	};
}
