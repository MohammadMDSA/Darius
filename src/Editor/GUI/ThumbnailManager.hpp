#pragma once

#include <ResourceManager/Resource.hpp>

#ifndef D_THUMBNAIL
#define D_THUMBNAIL Darius::Editor::Gui::ThumbnailManager
#endif // !D_THUMBNAIL

namespace Darius::Editor::Gui::ThumbnailManager
{

	enum class CommonIcon
	{
		Folder,
		File,

		NumIcons
	};

	void					Initialize();
	void					Shutdown();


	uint64_t GetIconTextureId(CommonIcon iconId);
	uint64_t GetResourceTextureId(D_RESOURCE::ResourceHandle const& resource);
	void RegisterExistingResources(D_FILE::Path const& path);
}
