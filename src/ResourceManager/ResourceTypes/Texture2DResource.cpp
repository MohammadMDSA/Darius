#include "ResourceManager/pch.hpp"
#include "Texture2DResource.hpp"

#include <Core/Filesystem/FileUtils.hpp>

namespace Darius::ResourceManager
{
	bool Texture2DResource::UploadToGpu(D_GRAPHICS::GraphicsContext& context)
	{
		auto path = GetPath();
		auto ext = path.extension().string();
		if (ext == ".dds")
		{
			auto fileData = D_FILE::ReadFileSync(path.wstring());
			return mTexture.CreateDDSFromMemory(fileData->data(), fileData->size(), true);
		}
		else if (ext == ".tga")
		{
			auto fileData = D_FILE::ReadFileSync(path.wstring());
			mTexture.CreateTGAFromMemory(fileData->data(), fileData->size(), true);

			return true;
		}

		return false;
	}

}