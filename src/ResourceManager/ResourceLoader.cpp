#include "pch.hpp"
#include "ResourceLoader.hpp"
#include "ResourceManager.hpp"
#include "ResourceTypes/MeshResource.hpp"
#include "ResourceTypes/MaterialResource.hpp"

#include <Core/Serialization/Json.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Utils/Common.hpp>

#include <fstream>

using namespace D_CORE;
using namespace D_SERIALIZATION;
namespace fs = std::filesystem;

namespace Darius::ResourceManager
{

	// Only used in resource reading / wrting, from / to file context
	ResourceHandle ResourceLoader::CreateResourceObject(ResourceMeta const& meta, DResourceManager* manager)
	{
		auto factory = Resource::GetFactoryForResourceType(meta.Type);
		if (!factory)
			return EmptyResourceHandle;

		return manager->CreateResource(meta.Type, meta.Uuid, meta.Path, false, true);
	}

	// Only used in resource reading / wrting, from / to file context
	ResourceHandle ResourceLoader::CreateResourceObject(Path const& path, DResourceManager* manager)
	{
		auto extension = path.extension();
		auto resourceType = Resource::GetResourceTypeByExtension(extension.string());

		return manager->CreateResource(resourceType, GenerateUuid(), path, false, true);
	}

	void SerializeMeta(Json& json, ResourceMeta const& meta)
	{
		json["Path"] = meta.Path.wstring();
		json["Name"] = meta.Name;
		json["Type"] = ResourceTypeToString(meta.Type);
		json["UUID"] = meta.Uuid.data;
	}

	void DeserializeMeta(Json const& json, ResourceMeta& meta)
	{
		meta.Path = json["Path"].get<std::wstring>();
		meta.Name = json["Name"].get<std::wstring>();
		meta.Type = StringToResourceType(json["Type"]);
		auto uuid = json["UUID"];

		for (size_t i = 0; i < 16; i++)
		{
			meta.Uuid.data[i] = uuid[i];
		}
	}

	bool ResourceLoader::SaveResource(Resource* resource)
	{
		return SaveResource(resource, false);
	}

	bool ResourceLoader::SaveResource(Resource* resource, bool metaOnly = false)
	{
		if (resource->GetType() == 0)
			throw D_CORE::Exception::Exception("Bad resource type to save");
		if (resource->mDefault)
		{
			resource->mDirtyDisk = false;
			return true;
		}

		ResourceMeta meta = *resource;

		Json jmeta;
		SerializeMeta(jmeta, meta);

		auto path = D_FILE::Path(resource->mPath.string() + ".tos");

		std::ofstream os(path);
		if (!os)
			int i = 3;
		os << jmeta;
		os.close();

		if (!metaOnly)
		{
			resource->WriteResourceToFile();
			resource->mDirtyDisk = false;
		}

		return true;
	}

	ResourceHandle ResourceLoader::LoadResource(Resource* resource)
	{

		if (resource->mDefault)
		{
			resource->mDirtyDisk = false;
			resource->mLoaded = true;
			return *resource;
		}

		return LoadResource(resource->GetPath());

	}

	ResourceHandle ResourceLoader::LoadResourceMeta(Path path)
	{
		// If already exists
		auto manager = D_RESOURCE::GetManager();
		if (manager->mPathMap.contains(path))
		{
			return *manager->mPathMap.at(path);
		}

		ResourceMeta meta;

		// Meta file exists?
		D_FILE::Path tosPath = D_FILE::Path(path).wstring() + L".tos";
		if (!D_H_ENSURE_FILE(tosPath))
			return EmptyResourceHandle;


		// Read from meta file
		std::ifstream is(tosPath);

		Json jMeta;
		is >> jMeta;
		is.close();

		DeserializeMeta(jMeta, meta);

		return CreateResourceObject(meta, manager);
	}

	ResourceHandle ResourceLoader::LoadResource(Path path, bool metaOnly)
	{
		if (!D_H_ENSURE_FILE(path))
			return EmptyResourceHandle;

		// Read meta
		auto handle = LoadResourceMeta(path);

		auto manager = D_RESOURCE::GetManager();

		// Meta available?
		if (handle.Type != 0)
		{
			// Fetch pointer to resource
			auto resource = manager->GetRawResource(handle);

			// Load if not loaded and should load, do it!
			if (!metaOnly && !resource->GetLoaded())
			{
				resource->ReadResourceFromFile();
				resource->mLoaded = true;
			}

			return *resource;

		}

		// No meta available for resource

		// Create resource object
		handle = CreateResourceObject(path, manager);

		// Resource not supported
		if (handle.Type == 0)
		{
			D_LOG_WARN("Resource " + path.filename().string() + " not supported");
			return handle;
		}

		// Fetch pointer to resource
		auto resource = _GetRawResource(handle);
		// Save meta to file
		SaveResource(resource, true);

		if (!metaOnly && !resource->GetLoaded())
		{
			resource->ReadResourceFromFile();
			resource->mLoaded = true;
		}

		return handle;
	}

	void ResourceLoader::VisitSubdirectory(Path path, bool recursively)
	{
		for (const auto& entry : fs::directory_iterator(path))
		{
			if (!entry.is_directory())
			{
				VisitFile(entry.path());
				continue;
			}
			else
			{
				if (recursively)
					VisitSubdirectory(entry.path());
			}
		}
	}

	void ResourceLoader::VisitFile(Path path)
	{
		if (path.extension() == ".tos")
			return;
		if (!D_H_ENSURE_FILE(path))
			throw D_EXCEPTION::FileNotFoundException("Resource on location " + path.string() + " not found");

		LoadResource(path, true);
	}

}
