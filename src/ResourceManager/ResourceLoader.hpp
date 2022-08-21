#pragma once

#include "ResourceTypes/Resource.hpp"

#include <Core/Filesystem/Path.hpp>

#ifndef D_RESOURCE_LOADER
#define D_RESOURCE_LOADER Darius::ResourceManager::ResourceLoader
#endif // !D_RESOURCE_LOADER


using namespace D_CORE;
using namespace D_FILE;

namespace Darius::ResourceManager
{
	class ResourceManager;

	class ResourceLoader
	{
	public:
		static bool				SaveResource(Resource* resource);

		static ResourceHandle	LoadResourceMeta(Path path);

		static ResourceHandle	LoadResource(Resource* resource);
		static ResourceHandle	LoadResource(Path path, bool metaOnly = false);

		static void				VisitSubdirectory(Path path, bool recursively = false);
		

	private:

		friend class ResourceManager;

		static bool				SaveResource(Resource* resource, bool metaOnly);
		static ResourceHandle	CreateResourceObject(ResourceMeta const& meta, DResourceManager* manager);
		static ResourceHandle	CreateResourceObject(Path const& path, DResourceManager* manager);
		static void				VisitFile(Path path);
	};
}
