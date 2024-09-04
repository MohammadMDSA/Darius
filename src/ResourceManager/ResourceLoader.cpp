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
			if(!mPendingToLoad)
			{
				if(mCallback)
					mCallback(nullptr);
				return;
			}

			if(!mPendingToLoad->IsLoaded())
				ResourceLoader::LoadResourceSync(mPendingToLoad);

			if(mUpdateGpu && mPendingToLoad->IsDirtyGPU())
				if(mPendingToLoad->UpdateGPU() == ResourceGpuUpdateResult::Success && mPendingToLoad->GetGpuState() != Resource::GPUDirtyState::Uploading)
					mPendingToLoad->MakeGpuClean();

			if(mCallback)
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

			if(mCallback)
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


		Resource* parentRes = nullptr;
		if(meta.Parent.Type != 0)
		{
			auto const& parent = meta.Parent;

			auto factory = Resource::GetFactoryForResourceType(parent.Type);
			D_ASSERT(factory);

			auto resouceHandle = manager->CreateResource(parent.Type, parent.Uuid, directory / meta.FileName, STR2WSTR(parent.Name), nullptr, false, true);

			if(resouceHandle.IsValid())
			{
				result.push_back(resouceHandle);
				parentRes = manager->GetRawResource(resouceHandle);
			}
		}

		for(auto resourceMeta : meta.Resources)
		{
			auto factory = Resource::GetFactoryForResourceType(resourceMeta.Type);
			if(!factory)
				continue;

			auto resouceHandle = manager->CreateResource(resourceMeta.Type, resourceMeta.Uuid, directory / meta.FileName, STR2WSTR(resourceMeta.Name), parentRes, false, true);

			if(resouceHandle.Type != 0)

				result.push_back(resouceHandle);
		}

		return result;
	}

	// Only used in resource reading/wrting, from/to file context
	DVector<ResourceHandle> ResourceLoader::CreateResourceObject(Path const& path, DResourceManager* manager)
	{
		auto extension = path.extension().string();
		auto extString = boost::algorithm::to_lower_copy(extension);
		auto resourceType = Resource::GetResourceTypeByExtension(extString);

		DVector<ResourceHandle> results;

		if(!resourceType.has_value())
			return results;

		auto resourcesToCreateFromProvider = D_RESOURCE::Resource::CanConstructTypeFromPath(resourceType.value(), path);

		Resource* parentRes = nullptr;
		D_ASSERT(resourcesToCreateFromProvider.Parent.Type != 0);
		if(resourcesToCreateFromProvider.Parent.Type != 0)
		{
			auto& parent = resourcesToCreateFromProvider.Parent;

			parent.Uuid = GenerateUuid();
			auto handle = manager->CreateResource(parent.Type, parent.Uuid, path, STR2WSTR(parent.Name), nullptr, false, true);

			if(handle.IsValid())
			{
				results.push_back(handle);
				parentRes = manager->GetRawResource(handle);
			}
		}

		for(auto resourceToCreate : resourcesToCreateFromProvider.SubResources)
		{
			resourceToCreate.Uuid = GenerateUuid();
			auto handle = manager->CreateResource(resourceToCreate.Type, resourceToCreate.Uuid, path, STR2WSTR(resourceToCreate.Name), parentRes, false, true);

			results.push_back(handle);
		}

		return results;

	}

	bool ResourceLoader::SaveResource(Resource* resource)
	{
		return SaveResource(resource, false);
	}

	bool ResourceLoader::SaveResource(Resource* resource, bool metaOnly = false)
	{
		if(resource->GetType() == 0)
			throw D_CORE::Exception::Exception("Bad resource type to save");

		/*if (resource->IsLocked())
			return false;*/

		if(resource->mDefault)
		{
			resource->SetLocked(true);
			resource->mDirtyDisk = false;
			resource->SetLocked(false);

			return true;
		}

		auto path = D_FILE::Path(resource->mPath.string() + ".tos");

		resource->SetLocked(true);

		Json resourceProps;
		if(!metaOnly)
		{
			resource->WriteResourceToFile(resourceProps);
			resource->mDirtyDisk = false;
		}

		// Meta already exists
		if(!D_H_ENSURE_FILE(path))
		{

			ResourceFileMeta meta = GetResourceFileMetaFromResource(resource);

			Json jmeta;
			jmeta = meta;

			jmeta["Properties"] = resourceProps;

			std::ofstream os(path);
			if(os)
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
			if(name.empty())
				name = L"__";;
			jmeta["Properties"][WSTR2STR(name)] = resourceProps;

			std::ofstream os(path);
			os << jmeta;
			os.close();
		}

		resource->SetLocked(false);

		return true;
	}

	ResourceHandle ResourceLoader::LoadResourceSync(Resource* resource, bool loadParent, bool forceLoad)
	{

		if(resource->mDefault)
		{
			resource->SetLocked(true);
			resource->mDirtyDisk = false;
			resource->mLoaded = true;
			resource->SetLocked(false);
			return *resource;
		}

		if(!forceLoad && resource->IsLoaded())
		{
			return *resource;
		}

		auto loaded = LoadResourceSync(resource->GetPath(), false, forceLoad, *resource);

		ResourceHandle resourceHandle = *resource;
		for(auto const& loadedHandle : loaded)
		{
			if(loadedHandle == resourceHandle)
				return resourceHandle;
		}

		return loaded.size() > 0 ? loaded[0] : EmptyResourceHandle;

	}

	void ResourceLoader::LoadResourceAsync(Resource* resource, ResourceLoadedResourceCalllback onLoaded, bool updateGpu)
	{
		if(resource == nullptr)
		{
			D_LOG_WARN("Trying to load a null resource");
			return;
		}

		if(resource->IsLoaded())
		{
			if(onLoaded)
				onLoaded(resource);
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
		if(manager->TryGetHandleFromPath(path, &existingResources))
		{
			foundMeta = true;
			alreadyExists = true;
		}

		ResourceFileMeta meta;

		// Meta file exists?
		D_FILE::Path tosPath = D_FILE::Path(path).wstring() + L".tos";
		if(!D_H_ENSURE_FILE(tosPath))
		{
			return {};
		}

		foundMeta = true;

		// Read from meta file
		std::ifstream is(tosPath);

		is >> jMeta;
		is.close();

		meta = jMeta;

		if(alreadyExists)
			return *existingResources;

		return CreateResourceObject(meta, manager, path.parent_path());
	}

	DVector<ResourceHandle> ResourceLoader::LoadResourceSync(Path const& path, bool metaOnly, bool forceLoad, ResourceHandle specificHandle, DVector<ResourceHandle> exclude)
	{
		if(!D_H_ENSURE_FILE(path))
			return { };

		// Read meta
		bool hasMeta;
		Json meta;
		auto handles = CreateReourceFromMeta(path, hasMeta, meta);

		auto manager = D_RESOURCE::GetManager();

		if(!hasMeta)
		{
			// No meta available for resource
			// Create resource object
			handles = CreateResourceObject(path, manager);
		}

		Json properties = meta.contains("Properties") ? meta["Properties"] : Json();

		bool specific = specificHandle.IsValid();

		for(auto handle : handles)
		{
			// Resource not supported
			if(handle.Type == 0 || std::find(exclude.begin(), exclude.end(), handle) != exclude.end())
				continue;
			/*if (specific && handle != specificHandle)
				continue;*/

				// Fetch pointer to resource
			auto resource = manager->GetRawResource(handle);

			// Already loaded?
			if(resource->IsLoaded() && !forceLoad)
				continue;

			resource->SetLocked(true);

			if(!hasMeta)
				// Save meta to file
				SaveResource(resource, true);

			bool dirtyDisk = false;

			// Load if not loaded and should load, do it!
			if(!metaOnly && !resource->IsLoaded())
			{
				auto resWName = resource->GetName();
				auto resName = WSTR2STR(resWName);
				if(resName.empty())
					resName = "__";
				resource->ReadResourceFromFile(properties.contains(resName) ? properties[resName] : Json(), dirtyDisk);
				resource->mLoaded = true;
			}

			resource->SetLocked(false);

			if(dirtyDisk)
				resource->MakeDiskDirty();
		}
		if(specific)
			return {specificHandle};

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
				if(pathName.starts_with(".") || pathName.starts_with("_"))
					return;

				if(isDir)
				{

					CheckDirectoryMeta(_path);
					if(recursively)
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

		Resource* parent = nullptr;
		D_CONTAINERS::DVector<Resource*> resources;

		for(auto resouceHandle : resourceHandleVec)
		{
			auto resource = manager->GetRawResource(resouceHandle);

			auto currentParent = resource->mParent;
			if(currentParent)
			{
				D_ASSERT(!parent || parent == currentParent);
				parent = currentParent;
			}

			resources.push_back(resource);
		}

		for(auto resource : resources)
		{
			ResourceDataInFile data;
			auto name = resource->GetName();
			data.Name = WSTR2STR(name);
			data.Type = resource->GetType();
			data.Uuid = resource->GetUuid();

			if(resource == parent)
				result.Parent = data;
			else
				result.Resources.push_back(data);
		}

		return result;
	}

	void ResourceLoader::CheckDirectoryMeta(Path const& path)
	{
		if(!D_H_ENSURE_DIR(path))
			return;

		auto parent = path.parent_path();
		auto name = path.filename().string();
		auto metaName = name + ".tos";

		auto metaPath = parent / metaName;

		if(D_H_ENSURE_FILE(metaPath))
			return;

		Json metaData;
		metaData["Path"] = name + "/";
		metaData["Folder"] = true;

		std::ofstream os(metaPath);
		if(os)
			os << metaData;
		os.close();
	}

	void ResourceLoader::VisitFile(Path const& path, DirectoryVisitProgress* progress)
	{
		if(path.extension() == ".tos")
			return;
		if(!D_H_ENSURE_FILE(path))
			throw D_EXCEPTION::FileNotFoundException("Resource on location " + path.string() + " not found");

		if(progress)
		{
			progress->Total++;
			LoadResourceAsync(path, [progress](auto _)
				{
					progress->Done++;
					if(progress->Done.load() % 100 == 0)
						D_LOG_INFO(progress->String("Updating resource database {} / {}"));
					if(progress->IsFinished())
					{
						if(progress->Done.load() % 100)
							D_LOG_INFO(progress->String("Updating resource database {} / {}"));
						{
							if(progress->OnFinish)
								progress->OnFinish();
							delete progress;
						}
					}

				}, true);
		}
	}

	void to_json(D_SERIALIZATION::Json& j, const ResourceFileMeta& value)
	{
		j["Path"] = WSTR2STR(value.FileName);
		if(!value.Parent.Uuid.is_nil())
			j["Parent"] = value.Parent;
		j["Resources"] = value.Resources;
	}

	void to_json(D_SERIALIZATION::Json& j, const ResourceDataInFile& value)
	{
		j["Name"] = value.Name;
		j["Type"] = Resource::GetResourceName(value.Type).string();
		j["Uuid"] = D_CORE::ToString(value.Uuid);
	}

	void from_json(const D_SERIALIZATION::Json& j, ResourceFileMeta& value)
	{
		std::string fname = j["Path"];
		value.FileName = STR2WSTR(fname);
		value.Resources = j["Resources"];

		if(j.contains("Parent"))
			value.Parent = j["Parent"];
	}

	void from_json(const D_SERIALIZATION::Json& j, ResourceDataInFile& value)
	{
		value.Name = j["Name"];
		value.Type = Resource::GetResourceTypeFromName(D_CORE::StringId(j["Type"].get<std::string>().c_str()));
		value.Uuid = D_CORE::FromString(j["Uuid"]);
	}
}
