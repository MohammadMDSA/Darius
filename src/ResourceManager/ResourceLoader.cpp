#include "pch.hpp"
#include "ResourceLoader.hpp"
#include "ResourceManager.hpp"

#include <Core/Serialization/Json.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Utils/Common.hpp>

#include <fstream>

using namespace D_CONTAINERS;
using namespace D_CORE;
using namespace D_FILE;
using namespace D_SERIALIZATION;

namespace Darius::ResourceManager
{

	// Only used in resource reading / wrting, from / to file context
	DVector<ResourceHandle> ResourceLoader::CreateResourceObject(ResourceFileMeta const& meta, DResourceManager* manager, Path const& directory)
	{
		auto result = DVector<ResourceHandle>();
		for (auto resourceMeta : meta.Resources)
		{
			auto factory = Resource::GetFactoryForResourceType(resourceMeta.Type);
			if (!factory)
				continue;

			auto resouceHandle = manager->CreateResource(resourceMeta.Type, resourceMeta.Uuid, directory / meta.FileName, WSTR_STR(resourceMeta.Name), false, true);

			if (resouceHandle.Type != 0)

				result.push_back(resouceHandle);
		}

		return result;
	}

	// Only used in resource reading/wrting, from/to file context
	DVector<ResourceHandle> ResourceLoader::CreateResourceObject(Path const& path, DResourceManager* manager)
	{
		auto extension = path.extension();
		auto resourceTypes = Resource::GetResourceTypeByExtension(extension.string());

		auto results = DVector<ResourceHandle>();

		for (auto type : resourceTypes)
		{
			auto resourcesToCreateFromProvider = D_RESOURCE::Resource::CanConstructTypeFromPath(type, path);

			for (auto resourceToCreate : resourcesToCreateFromProvider)
			{
				resourceToCreate.Uuid = GenerateUuid();
				auto handle = manager->CreateResource(resourceToCreate.Type, resourceToCreate.Uuid, path, WSTR_STR(resourceToCreate.Name), false, true);
				results.push_back(handle);

			}

		}

		return results;

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

		auto path = D_FILE::Path(resource->mPath.string() + ".tos");

		Json resourceProps;
		if (!metaOnly)
		{
			resource->WriteResourceToFile(resourceProps);
			resource->mDirtyDisk = false;
		}

		// Meta already exists
		if (!D_H_ENSURE_FILE(path))
		{

			ResourceFileMeta meta = GetResourceFileMetaFromResource(resource);

			Json jmeta;
			jmeta = meta;

			jmeta["Properties"] = resourceProps;

			std::ofstream os(path);
			if (os)
				os << jmeta;
			os.close();
		}
		else
		{
			std::ifstream is(path);

			Json jmeta;
			is >> jmeta;
			is.close();

			auto name = resource->GetName();
			jmeta["Properties"][STR_WSTR(name)] = resourceProps;

			std::ofstream os(path);
			os << jmeta;
			os.close();
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

		auto loaded = LoadResource(resource->GetPath());

		ResourceHandle resourceHandle = *resource;
		for (auto const& loadedHandle : loaded)
		{
			if (loadedHandle == resourceHandle)
				return resourceHandle;
		}

		return loaded.size() > 0 ? loaded[0] : EmptyResourceHandle;

	}

	DVector<ResourceHandle> ResourceLoader::CreateReourceFromMeta(Path const& _path, bool& foundMeta, Json& jMeta)
	{
		foundMeta = false;
		auto path = _path.lexically_normal();

		bool alreadyExists = false;

		// If already exists
		auto manager = D_RESOURCE::GetManager();
		if (manager->mPathMap.contains(path))
		{
			foundMeta = true;
			alreadyExists = true;
		}

		ResourceFileMeta meta;

		// Meta file exists?
		D_FILE::Path tosPath = D_FILE::Path(path).wstring() + L".tos";
		if (!D_H_ENSURE_FILE(tosPath))
		{
			return {};
		}

		foundMeta = true;

		// Read from meta file
		std::ifstream is(tosPath);

		is >> jMeta;
		is.close();

		meta = jMeta;

		if (alreadyExists)
			return manager->mPathMap.at(path);

		return CreateResourceObject(meta, manager, path.parent_path());
	}

	DVector<ResourceHandle> ResourceLoader::LoadResource(Path const& path, bool metaOnly)
	{
		if (!D_H_ENSURE_FILE(path))
			return { };

		// Read meta
		bool hasMeta;
		Json meta;
		auto handles = CreateReourceFromMeta(path, hasMeta, meta);

		auto manager = D_RESOURCE::GetManager();

		if (!hasMeta)
		{
			// No meta available for resource
			// Create resource object
			handles = CreateResourceObject(path, manager);
		}

		Json properties = meta.contains("Properties") ? meta["Properties"] : Json();

		for (auto handle : handles)
		{
			// Resource not supported
			if (handle.Type != 0)
			{
				// Fetch pointer to resource
				auto resource = manager->GetRawResource(handle);

				if (!hasMeta)
					// Save meta to file
					SaveResource(resource, true);

				// Load if not loaded and should load, do it!
				if (!metaOnly && !resource->IsLoaded())
				{
					auto resWName = resource->GetName();
					auto resName = STR_WSTR(resWName);
					resource->ReadResourceFromFile(properties.contains(resName) ? properties[resName] : Json());
					resource->mLoaded = true;
				}

			}
		}
		return handles;
	}

	void ResourceLoader::VisitSubdirectory(Path const& path, bool recursively)
	{
		D_FILE::VisitEntriesInDirectory(path, false, [&](Path const& _path, bool isDir)
			{
				if (isDir)
				{
					CheckDirectoryMeta(_path);
					if (recursively)
						VisitSubdirectory(_path, true);
				}
				else
					VisitFile(_path);
			});
	}

	ResourceFileMeta ResourceLoader::GetResourceFileMetaFromResource(Resource* resource)
	{
		auto const& path = resource->GetPath();
		auto pathStr = path.lexically_normal().wstring();

		auto manager = D_RESOURCE::GetManager();
		auto resourceHandleVec = manager->mPathMap[pathStr];

		ResourceFileMeta result;

		result.FileName = path.filename();

		for (auto resouceHandle : resourceHandleVec)
		{
			auto resource = manager->GetRawResource(resouceHandle);

			ResourceDataInFile data;
			auto name = resource->GetName();
			data.Name = STR_WSTR(name);
			data.Type = resource->GetType();
			data.Uuid = resource->GetUuid();

			result.Resources.push_back(data);

		}

		return result;
	}

	void ResourceLoader::CheckDirectoryMeta(Path const& path)
	{
		if (!D_H_ENSURE_DIR(path))
			return;

		auto parent = path.parent_path();
		auto name = path.filename().string();
		auto metaName = name + ".tos";

		auto metaPath = parent / metaName;

		if (D_H_ENSURE_FILE(metaPath))
			return;

		Json metaData;
		metaData["Path"] = name + "/";
		metaData["Folder"] = true;

		std::ofstream os(metaPath);
		if (os)
			os << metaData;
		os.close();
	}

	void ResourceLoader::VisitFile(Path const& path)
	{
		if (path.extension() == ".tos")
			return;
		if (!D_H_ENSURE_FILE(path))
			throw D_EXCEPTION::FileNotFoundException("Resource on location " + path.string() + " not found");

		LoadResource(path, true);
	}

	void to_json(D_SERIALIZATION::Json& j, const ResourceFileMeta& value)
	{
		j["Path"] = STR_WSTR(value.FileName);
		j["Resources"] = value.Resources;
	}

	void to_json(D_SERIALIZATION::Json& j, const ResourceDataInFile& value)
	{
		j["Name"] = value.Name;
		j["Type"] = Resource::GetResourceName(value.Type);
		j["Uuid"] = D_CORE::ToString(value.Uuid);
	}

	void from_json(const D_SERIALIZATION::Json& j, ResourceFileMeta& value)
	{
		std::string fname = j["Path"];
		value.FileName = WSTR_STR(fname);
		value.Resources = j["Resources"];
	}

	void from_json(const D_SERIALIZATION::Json& j, ResourceDataInFile& value)
	{
		value.Name = j["Name"];
		value.Type = Resource::GetResourceTypeFromName(j["Type"].get<std::string>());
		value.Uuid = D_CORE::FromString(j["Uuid"]);
	}
}
