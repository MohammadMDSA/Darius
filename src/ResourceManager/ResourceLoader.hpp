#pragma once

#include "Resource.hpp"

#include <Core/Filesystem/Path.hpp>

#ifndef D_RESOURCE_LOADER
#define D_RESOURCE_LOADER Darius::ResourceManager::ResourceLoader
#endif // !D_RESOURCE_LOADER


namespace Darius::ResourceManager
{
	class ResourceManager;

	class ResourceLoader
	{
	public:
		static bool				SaveResource(Resource* resource);

		static D_CONTAINERS::DVector<ResourceHandle> CreateReourceFromMeta(D_FILE::Path const& path, bool& foundMeta, D_SERIALIZATION::Json& jMeta);

		static ResourceHandle	LoadResource(Resource* resource);
		static D_CONTAINERS::DVector<ResourceHandle>	LoadResource(D_FILE::Path const& path, bool metaOnly = false);

		static void				VisitSubdirectory(D_FILE::Path const& path, bool recursively = false);
		static ResourceFileMeta GetResourceFileMetaFromResource(Resource* resource);

		static INLINE D_FILE::Path GetPathForNewResource(std::wstring const& name, std::wstring const& ext, D_FILE::Path const& parent) { auto dir = D_FILE::Path(parent); return dir.append(D_FILE::GetNewFileName(name, ext, dir)); }

	private:

		friend class ResourceManager;

		static bool				SaveResource(Resource* resource, bool metaOnly);
		static D_CONTAINERS::DVector<ResourceHandle> CreateResourceObject(ResourceFileMeta const& meta, DResourceManager* manager, D_FILE::Path const& directory);
		static D_CONTAINERS::DVector<ResourceHandle> CreateResourceObject(D_FILE::Path const& path, DResourceManager* manager);
		static void				VisitFile(D_FILE::Path const& path);
		static void				CheckDirectoryMeta(D_FILE::Path const& path);
	};

	void to_json(D_SERIALIZATION::Json& j, const ResourceFileMeta& value);
	void to_json(D_SERIALIZATION::Json& j, const ResourceDataInFile& value);
	void from_json(const D_SERIALIZATION::Json& j, ResourceFileMeta& value);
	void from_json(const D_SERIALIZATION::Json& j, ResourceDataInFile& value);
}
