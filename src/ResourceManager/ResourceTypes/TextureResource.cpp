#include "ResourceManager/pch.hpp"
#include "TextureResource.hpp"

#include "ResourceManager/ResourceManager.hpp"

namespace Darius::ResourceManager
{
	void TextureResource::WriteResourceToFile() const
	{
	}

	void TextureResource::ReadResourceFromFile()
	{
	}

	void TextureResource::Unload()
	{
		EvictFromGpu();
	}

}
