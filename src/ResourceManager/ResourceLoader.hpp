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

		static DVector<ResourceHandle> CreateReourceFromMeta(Path path, bool& foundMeta);

		static ResourceHandle	LoadResource(Resource* resource);
		static DVector<ResourceHandle>	LoadResource(Path path, bool metaOnly = false);

		static void				VisitSubdirectory(Path path, bool recursively = false);
		static ResourceFileMeta GetResourceFileMetaFromResource(Resource* resource);

	private:

		friend class ResourceManager;

		static bool				SaveResource(Resource* resource, bool metaOnly);
		static DVector<ResourceHandle> CreateResourceObject(ResourceFileMeta const& meta, DResourceManager* manager);
		static DVector<ResourceHandle> CreateResourceObject(Path const& path, DResourceManager* manager);
		static void				VisitFile(Path path);
	};

	void to_json(D_SERIALIZATION::Json& j, const ResourceFileMeta& value);
	void to_json(D_SERIALIZATION::Json& j, const ResourceDataInFile& value);
	void from_json(const D_SERIALIZATION::Json& j, ResourceFileMeta& value);
	void from_json(const D_SERIALIZATION::Json& j, ResourceDataInFile& value);
}
