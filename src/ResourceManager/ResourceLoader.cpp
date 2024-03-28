#include "pch.hpp"
#include "ResourceLoader.hpp"
#include "ResourceManager.hpp"

#include <Core/Serialization/Json.hpp>
#include <Core/Containers/ConcurrentQueue.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Job/Job.hpp>
#include <Utils/Common.hpp>
#include <Utils/Log.hpp>

#include <boost/algorithm/string.hpp>

#include <concurrent_vector.h>
#include <fstream>

using namespace D_CONTAINERS;
using namespace D_CORE;
using namespace D_FILE;
using namespace D_SERIALIZATION;

namespace Darius::ResourceManager
{
	struct AsyncResourceLoadingFromResourceDataTask : public D_JOB::IPinnedTask
	{
		virtual void Execute() override
		{
			if (!mPendingToLoad)
			{
				if (mCallback)
					mCallback(nullptr);
				return;
			}

			if (!mPendingToLoad->IsLoaded())
				ResourceLoader::LoadResourceSync(mPendingToLoad);

			if (mUpdateGpu && mPendingToLoad->IsDirtyGPU())
				if (mPendingToLoad->UpdateGPU() == ResourceGpuUpdateResult::Success)
					mPendingToLoad->MakeGpuClean();

			if (mCallback)
				mCallback(mPendingToLoad);
		}

		Resource* mPendingToLoad = nullptr;
		bool							mUpdateGpu = false;
		ResourceLoadedResourceCalllback	mCallback = nullptr;
	};

	struct AsyncResourceLoadingFromPathTask : public D_JOB::IPinnedTask
	{
		virtual void Execute() override
		{
			auto loaded = ResourceLoader::LoadResourceSync(mPath, mMetaOnly);

			if (mCallback)
				mCallback(loaded);

		}

		D_FILE::Path					mPath;
		bool							mMetaOnly = false;
		ResourceLoadedResourceListCalllback	mCallback = nullptr;
	};

	// Only used in resource reading / wrting, from / to file context
	DVector<ResourceHandle> ResourceLoader::CreateResourceObject(ResourceFileMeta const& meta, DResourceManager* manager, Path const& directory)
	{
		auto result = DVector<ResourceHandle>();
		for (auto resourceMeta : meta.Resources)
		{
			auto factory = Resource::GetFactoryForResourceType(resourceMeta.Type);
			if (!factory)
				continue;

			auto resouceHandle = manager->CreateResource(resourceMeta.Type, resourceMeta.Uuid, directory / meta.FileName, STR2WSTR(resourceMeta.Name), false, true);

			if (resouceHandle.Type != 0)

				result.push_back(resouceHandle);
		}

		return result;
	}

	// Only used in resource reading/wrting, from/to file context
	DVector<ResourceHandle> ResourceLoader::CreateResourceObject(Path const& path, DResourceManager* manager)
	{
		auto extension = path.extension().string();
		auto extString = boost::algorithm::to_lower_copy(extension);
		auto resourceTypes = Resource::GetResourceTypeByExtension(extString);
		auto resTypesVec = D_CONTAINERS::DVector<ResourceType>(resourceTypes.begin(), resourceTypes.end());

		DVector<ResourceHandle> results;
		std::mutex mutex;

		D_JOB::AddTaskSetAndWait((UINT)resourceTypes.size(), [&results, &resTypesVec, &path, manager, &mutex](D_JOB::TaskPartition range, D_JOB::ThreadNumber threadNumber)
			{
				for (int i = range.start; i < range.end; i++)
				{
					auto type = resTypesVec.at(i);
					auto resourcesToCreateFromProvider = D_RESOURCE::Resource::CanConstructTypeFromPath(type, path);

					for (auto resourceToCreate : resourcesToCreateFromProvider)
					{
						resourceToCreate.Uuid = GenerateUuid();
						auto handle = manager->CreateResource(resourceToCreate.Type, resourceToCreate.Uuid, path, STR2WSTR(resourceToCreate.Name), false, true);

						{
							std::scoped_lock lock(mutex);
							results.push_back(handle);
						}

					}
				}
			});


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

		/*if (resource->IsLocked())
			return false;*/

		if (resource->mDefault)
		{
			resource->SetLocked(true);
			resource->mDirtyDisk = false;
			resource->SetLocked(false);

			return true;
		}

		auto path = D_FILE::Path(resource->mPath.string() + ".tos");

		resource->SetLocked(true);

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
			jmeta["Properties"][WSTR2STR(name)] = resourceProps;

			std::ofstream os(path);
			os << jmeta;
			os.close();
		}

		resource->SetLocked(false);

		return true;
	}

	ResourceHandle ResourceLoader::LoadResourceSync(Resource* resource)
	{

		if (resource->mDefault)
		{
			resource->SetLocked(true);
			resource->mDirtyDisk = false;
			resource->mLoaded = true;
			resource->SetLocked(false);
			return *resource;
		}

		auto loaded = LoadResourceSync(resource->GetPath(), false, *resource);

		ResourceHandle resourceHandle = *resource;
		for (auto const& loadedHandle : loaded)
		{
			if (loadedHandle == resourceHandle)
				return resourceHandle;
		}

		return loaded.size() > 0 ? loaded[0] : EmptyResourceHandle;

	}

	void ResourceLoader::LoadResourceAsync(Resource* resource, ResourceLoadedResourceCalllback onLoaded, bool updateGpu)
	{
		if (resource == nullptr)
		{
			D_LOG_WARN("Trying to load a null resource");
			return;
		}

		auto task = new AsyncResourceLoadingFromResourceDataTask();
		task->mCallback = onLoaded;
		task->mPendingToLoad = resource;
		task->mUpdateGpu = updateGpu;

		D_JOB::AddPinnedTask(task, D_JOB::ThreadType::FileIO);
	}


	DVector<ResourceHandle> ResourceLoader::CreateReourceFromMeta(Path const& _path, bool& foundMeta, Json& jMeta)
	{
		foundMeta = false;
		auto path = _path.lexically_normal();

		bool alreadyExists = false;

		// If already exists
		auto manager = D_RESOURCE::GetManager();

		DVector<ResourceHandle>const* existingResources;
		if (manager->TryGetHandleFromPath(path, &existingResources))
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
			return *existingResources;

		return CreateResourceObject(meta, manager, path.parent_path());
	}

	DVector<ResourceHandle> ResourceLoader::LoadResourceSync(Path const& path, bool metaOnly, ResourceHandle specificHandle)
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

		bool specific = specificHandle.IsValid();

		for (auto handle : handles)
		{
			// Resource not supported
			if (handle.Type != 0)
			{
				if (specific && handle != specificHandle)
					continue;

				// Fetch pointer to resource
				auto resource = manager->GetRawResource(handle);

				resource->SetLocked(true);

				if (!hasMeta)
					// Save meta to file
					SaveResource(resource, true);

				bool dirtyDisk = false;

				// Load if not loaded and should load, do it!
				if (!metaOnly && !resource->IsLoaded())
				{
					auto resWName = resource->GetName();
					auto resName = WSTR2STR(resWName);
					resource->ReadResourceFromFile(properties.contains(resName) ? properties[resName] : Json(), dirtyDisk);
					resource->mLoaded = true;
				}

				resource->SetLocked(false);

				if (dirtyDisk)
					resource->MakeDiskDirty();
			}
		}
		if (specific)
			return { specificHandle };

		return handles;
	}

	void ResourceLoader::LoadResourceAsync(D_FILE::Path const& path, ResourceLoadedResourceListCalllback onLoaded, bool metaOnly)
	{
		auto task = new AsyncResourceLoadingFromPathTask();
		task->mCallback = onLoaded;
		task->mPath = path;
		task->mMetaOnly = metaOnly;

		D_JOB::AddPinnedTask(task, D_JOB::ThreadType::FileIO);
	}


	void ResourceLoader::VisitSubdirectory(Path const& path, bool recursively, DirectoryVisitProgress* progress)
	{
		D_FILE::VisitEntriesInDirectory(path, false, [&](Path const& _path, bool isDir)
			{
				auto pathName = _path.filename().string();
				if (pathName.starts_with(".") || pathName.starts_with("_"))
					return;

				if (isDir)
				{

					CheckDirectoryMeta(_path);
					if (recursively)
						VisitSubdirectory(_path, true, progress);
				}
				else
					VisitFile(_path, progress);
			});
	}

	ResourceFileMeta ResourceLoader::GetResourceFileMetaFromResource(Resource* resource)
	{
		auto const& path = resource->GetPath();
		auto pathStr = path.lexically_normal().wstring();

		auto manager = D_RESOURCE::GetManager();
		auto resourceHandleVec = manager->GetHandleFromPath(pathStr);

		ResourceFileMeta result;

		result.FileName = path.filename();

		for (auto resouceHandle : resourceHandleVec)
		{
			auto resource = manager->GetRawResource(resouceHandle);

			ResourceDataInFile data;
			auto name = resource->GetName();
			data.Name = WSTR2STR(name);
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

	void ResourceLoader::VisitFile(Path const& path, DirectoryVisitProgress* progress)
	{
		if (path.extension() == ".tos")
			return;
		if (!D_H_ENSURE_FILE(path))
			throw D_EXCEPTION::FileNotFoundException("Resource on location " + path.string() + " not found");

		if (progress)
		{
			progress->Total++;
			LoadResourceAsync(path, [progress](auto _)
				{
					progress->Done++;
					if (progress->Done.load() % 100 == 0)
						D_LOG_INFO(progress->String("Updating resource database {} / {}"));
					if (progress->IsFinished())
					{
						if (progress->Done.load() % 100)
							D_LOG_INFO(progress->String("Updating resource database {} / {}"));
						delete progress;
					}

				}, true);
		}
	}

	void to_json(D_SERIALIZATION::Json& j, const ResourceFileMeta& value)
	{
		j["Path"] = WSTR2STR(value.FileName);
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
		value.FileName = STR2WSTR(fname);
		value.Resources = j["Resources"];
	}

	void from_json(const D_SERIALIZATION::Json& j, ResourceDataInFile& value)
	{
		value.Name = j["Name"];
		value.Type = Resource::GetResourceTypeFromName(j["Type"].get<std::string>());
		value.Uuid = D_CORE::FromString(j["Uuid"]);
	}
}
