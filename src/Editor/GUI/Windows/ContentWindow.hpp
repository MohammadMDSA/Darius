#pragma once

#include "Window.hpp"
#include "Editor/GUI/Components/ContentWindowComponents.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Containers/Vector.hpp>

namespace Darius::Editor::Gui::Windows
{
	class ContentWindow : public Window
	{
	public:
		ContentWindow();
		~ContentWindow();

		// Inherited via Window
		INLINE virtual std::string const GetName() override { return "Content"; }

		INLINE virtual void			Render(D_GRAPHICS::GraphicsContext&) override {}

		INLINE virtual void			Update(float) override {}

		virtual void				DrawGUI() override;

		void						UpdateDirectoryItems();
		bool						SetCurrentPath(D_FILE::Path const& path);

		D_CH_R_FIELD(D_FILE::Path, CurrentDirectory);

	private:
		void						DrawMainItems();
		void						DrawBreadcrumb();

		D_CONTAINERS::DUnorderedMap<D_RESOURCE::ResourceType, uint64_t> mResourceTypeTextureMap;
		
		D_CONTAINERS::DVector<D_GUI_COMPONENT::EditorContentWindowItem>	mCurrentDirectoryItems;
		D_CONTAINERS::DVector<D_FILE::Path> mBreadcrumbItems; // it's from child to parent

	};
}
