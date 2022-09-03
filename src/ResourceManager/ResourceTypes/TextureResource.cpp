#include "ResourceManager/pch.hpp"
#include "TextureResource.hpp"

#include "ResourceManager/ResourceManager.hpp"

namespace Darius::ResourceManager
{
	bool TextureResource::SuppoertsExtension(std::wstring ext)
	{
		if (ext == L".tga" || ext == L".dds")
			return true;
		return false;
	}

	void TextureResource::WriteResourceToFile() const
	{
	}

	void TextureResource::ReadResourceFromFile()
	{
	}

}
