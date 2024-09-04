#include "pch.hpp"
#include "ResourceManager.hpp"
#include "Resource.hpp"
#include "ResourceLoader.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Filesystem/FileUtils.hpp>
#include <Core/Containers/Map.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Job/Job.hpp>
#include <Utils/Assert.hpp>

using namespace D_CONTAINERS;
using namespace D_CORE;
using namespace D_FILE;

namespace Darius::ResourceManager
{
	bool _initialized = false;

	std::unique_ptr<DResourceManager>				_ResourceManager;

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(_ResourceManager == nullptr);

		_ResourceManager = std::make_unique<DResourceManager>();

	}

	void Shutdown()
	{
		D_ASSERT(_ResourceManager);
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		return false;
	}
#endif

	DResourceManager* GetManager()
	{
		return _ResourceManager.get();
	}

	Resource* GetRawResourceSync(D_CORE::Uuid const& uuid, bool syncLoad)
	{
		auto resource = _ResourceManager->GetRawResource(uuid);

		if (!resource)
			return nullptr;

		// Load resource if not loaded yet
		if(syncLoad && !resource->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceSync(resource);
		if(syncLoad && resource->IsDirtyGPU())
		{
			if(resource->UpdateGPU() == ResourceGpuUpdateResult::Success && resource->GetGpuState() != Resource::GPUDirtyState::Uploading)
				resource->MakeGpuClean();
		}
		return resource;
	}

	Resource* GetRawResourceSync(ResourceHandle handle, bool syncLoad)
	{
		auto resource = _ResourceManager->GetRawResource(handle);

		// Load resource if not loaded yet
		if(syncLoad && !resource->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceSync(resource);
		if(syncLoad && resource->IsDirtyGPU())
		{
			if(resource->UpdateGPU() == ResourceGpuUpdateResult::Success && resource->GetGpuState() != Resource::GPUDirtyState::Uploading)
				resource->MakeGpuClean();
		}
		return resource;
	}

	void GetRawResourceAsync(ResourceHandle handle, ResourceLoadedResourceCalllback callback)
	{
		auto resource = _ResourceManager->GetRawResource(handle);

		ResourceLoader::LoadResourceAsync(resource, callback, true);
	}

	void GetRawResourceAsync(D_CORE::Uuid const& uuid, ResourceLoadedResourceCalllback callback)
	{
		auto resource = _ResourceManager->GetRawResource(uuid);

		ResourceLoader::LoadResourceAsync(resource, callback, true);
	}

	ResourceHandle GetResourceHandle(D_CORE::Uuid const& uuid)
	{
		auto resource = _ResourceManager->GetRawResource(uuid);
		if(!resource)
			return EmptyResourceHandle;

		return *resource;
	}

#ifdef _D_EDITOR
	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type)
	{
		return _ResourceManager->GetResourcePreviews(type);
	}

	ResourcePreview GetResourcePreview(ResourceHandle const& handle)
	{
		return _ResourceManager->GetResourcePreview(handle);
	}
#endif

	void UpdateGPUResources()
	{
		_ResourceManager->UpdateGPUResources();
	}

	void SaveAll()
	{
		_ResourceManager->SaveAllResources();
	}

#ifdef _D_EDITOR
	void GetAllResources(DVector<Resource*>& resources)
	{
		_ResourceManager->GetAllResources(resources);
	}

	void GetAllResourcePaths(DVector<Path>& paths)
	{
		_ResourceManager->GetAllResourcePaths(paths);
	}

	void GetAllResourcePathsInDirectory(D_FILE::Path const& parentDirectory, D_CONTAINERS::DVector<D_FILE::Path>& paths)
	{
		DVector<Path> pathsInternal;
		paths.clear();
		_ResourceManager->GetAllResourcePaths(pathsInternal);
		std::copy_if(pathsInternal.begin(), pathsInternal.end(), std::back_inserter(paths), [parentDirectory](D_FILE::Path const& el)
			{
				if(!el.has_parent_path())
					return false;
				return std::filesystem::equivalent(parentDirectory, el.parent_path());
			});
	}

#endif // _D_EDITOR


	DResourceManager::DResourceManager() :
		mDefaultResourcesSet()
	{ }

	DResourceManager::~DResourceManager()
	{
		mDefaultResourcesSet.clear();

		for(auto& typeClass : mResourceMap)
			for(auto& resourcePair : typeClass.second)
				resourcePair.second->Destroy();
	}

	Resource* DResourceManager::GetRawResource(D_CORE::Uuid const& uuid)
	{
		if(mUuidMap.find(uuid) == mUuidMap.end())
			return nullptr;
		return mUuidMap[uuid];
	}

	Resource* DResourceManager::GetRawResource(ResourceHandle handle)
	{
		if(handle.Type == 0)
			return nullptr;

		if(!D_VERIFY(mResourceMap.find(handle.Type) != mResourceMap.end()))
			return nullptr;
		auto& typeClass = mResourceMap[handle.Type];
		if(!D_VERIFY(typeClass.contains(handle.Id)))
			return nullptr;
		return typeClass[handle.Id].get();
	}

	Resource* DResourceManager::GetRawResourceSafe(ResourceHandle handle)
	{
		if(handle.Type == 0)
			return nullptr;

		if(mResourceMap.find(handle.Type) == mResourceMap.end())
			return nullptr;
		auto& typeClass = mResourceMap[handle.Type];
		if(!typeClass.contains(handle.Id))
			return nullptr;
		return typeClass[handle.Id].get();
	}

#ifdef _D_EDITOR
	DVector<ResourcePreview> DResourceManager::GetResourcePreviews(ResourceType type)
	{
		DVector<ResourcePreview> res;
		res.reserve(mResourceMap.at(type).size());
		for(auto& resource : mResourceMap.at(type))
		{
			res.push_back(*resource.second);
		}

		std::sort(res.begin(), res.end(), [](ResourcePreview const& a, ResourcePreview const& b)
			{
				return a.Name < b.Name;
			});

		return res;
	}

	ResourcePreview DResourceManager::GetResourcePreview(ResourceHandle const& handle)
	{
		auto res = GetRawResourceSafe(handle);
		if(res == nullptr)
			return {L"", D_FILE::Path(), EmptyResourceHandle};

		return *res;
	}
#endif

	ResourceHandle DResourceManager::CreateResource(ResourceType type, Uuid const& uuid, std::wstring const& path, std::wstring const& name, Resource* parent, bool isDefault, bool fromFile)

	{
		if(!fromFile && D_H_ENSURE_FILE(path))
			throw D_EXCEPTION::Exception(("A file with the same name already exists: " + WSTR2STR(path)).c_str());

		auto factory = Resource::GetFactoryForResourceType(type);
		if(!factory)
			return EmptyResourceHandle;

		auto res = factory->Create(uuid, path, name, GetNewId(), parent, isDefault);

		if(isDefault)
			mDefaultResourcesSet.push_back(res.get());

		res->mLoaded = !fromFile;
		res->mDirtyDisk = !fromFile;

		// Add the handle to path and resource maps
		ResourceHandle handle = {type , res->GetId()};
		UpdateMaps(res);

		return handle;
	}

	void DResourceManager::UpdateGPUResources()
	{
		static DVector<Resource*> dirtyResources;
		static DVector<Resource*> stillDirtyResources;
		dirtyResources.clear();
		stillDirtyResources.clear();

		// Iterating over all resources
		for(auto& resType : mResourceMap)
		{
			for(auto& res : resType.second)
			{
				auto resource = res.second;
				if(resource->IsLoaded() && resource->GetGpuState() == Resource::GPUDirtyState::Dirty && !resource->IsLocked())
				{
					switch(resource->UpdateGPU())
					{
						// It is successfully cleaned
					case ResourceGpuUpdateResult::Success:
						if(resource->GetGpuState() == Resource::GPUDirtyState::Dirty)
							dirtyResources.push_back(resource.get());
						break;

						// It will be cleaned in the next round
					case ResourceGpuUpdateResult::DirtyDependency:
						stillDirtyResources.push_back(resource.get());
						break;

					case ResourceGpuUpdateResult::AlreadyClean:
					default:
						break;
					}
				}
			}
		}

		// Make gpu state of the clean ones, clean
		for(auto resource : dirtyResources)
			resource->MakeGpuClean();
		dirtyResources.clear();

		while(!stillDirtyResources.empty())
		{
			UINT stillDirtyCount = (UINT)stillDirtyResources.size();
			static DVector<Resource*> newStillDirtyResources;

			for(auto resource : stillDirtyResources)
			{
				switch(resource->UpdateGPU())
				{
					// It is successfully cleaned
				case ResourceGpuUpdateResult::Success:
					dirtyResources.push_back(resource);
					break;

					// It will be cleaned in the next round
				case ResourceGpuUpdateResult::DirtyDependency:
					newStillDirtyResources.push_back(resource);
					break;

				case ResourceGpuUpdateResult::AlreadyClean:
				default:
					break;
				}
			}

			// Make gpu state of the clean ones, clean
			for(auto resource : dirtyResources)
				resource->MakeGpuClean();
			dirtyResources.clear();

			stillDirtyResources = newStillDirtyResources;
			newStillDirtyResources.clear();
			if((UINT)stillDirtyResources.size() == stillDirtyCount)
				break;
		}
	}

	void DResourceManager::UpdateMaps(std::shared_ptr<Resource> resource)
	{

		// Update resource map
		mResourceMap.at(resource->GetType()).try_emplace(resource->GetId(), resource);

		// Update uuid map
		mUuidMap.insert({resource->GetUuid(), resource.get()});

		// Update path map

		auto path = resource->GetPath().lexically_normal().wstring();

		auto& pathHandles = mPathMap[path];
		bool found = false;
		ResourceHandle newHandle = *resource;
		for(auto const& handle : pathHandles)
		{
			if(handle.Id == newHandle.Id && handle.Type == newHandle.Type)
			{
				found = true;
				break;
			}
		}
		if(!found)
			pathHandles.push_back(newHandle);
	}

	void DResourceManager::SaveAllResources()
	{
		for(auto& resType : mResourceMap)
		{
			for(auto& res : resType.second)
			{
				auto resource = res.second;
				if(resource->IsDirtyDisk())
					D_RESOURCE_LOADER::SaveResource(resource.get());
			}
		}
	}

#ifdef _D_EDITOR
	void DResourceManager::GetAllResources(DVector<Resource*>& resources) const
	{
		for(auto& resType : mResourceMap)
		{
			for(auto& res : resType.second)
			{
				resources.push_back(res.second.get());
			}
		}
	}

	void DResourceManager::GetAllResourcePaths(DVector<Path>& paths) const
	{
		for(auto const& path : mPathMap)
		{
			paths.push_back(Path(path.first));
		}
	}
#endif // _D_EDITOR
}