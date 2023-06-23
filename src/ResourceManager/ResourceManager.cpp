#include "pch.hpp"
#include "ResourceManager.hpp"
#include "Resource.hpp"
#include "ResourceLoader.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Filesystem/FileUtils.hpp>
#include <Core/Containers/Map.hpp>
#include <Core/Exceptions/Exception.hpp>
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

	Resource* _GetRawResource(D_CORE::Uuid const& uuid, bool load)
	{
		auto resource = _ResourceManager->GetRawResource(uuid);

		// Load resource if not loaded yet
		if (load && !resource->IsLoaded())
			D_RESOURCE_LOADER::LoadResource(resource);
		if (load && resource->IsDirtyGPU())
		{
			resource->UpdateGPU();
		}
		return resource;
	}

	Resource* _GetRawResource(ResourceHandle handle, bool load)
	{
		auto resource = _ResourceManager->GetRawResource(handle);

		// Load resource if not loaded yet
		if (load && !resource->IsLoaded())
			D_RESOURCE_LOADER::LoadResource(resource);
		if (load && resource->IsDirtyGPU())
		{
			resource->UpdateGPU();
		}
		return resource;
	}

	ResourceHandle GetResourceHandle(D_CORE::Uuid const& uuid)
	{
		return *_ResourceManager->GetRawResource(uuid);
	}

	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type)
	{
		return _ResourceManager->GetResourcePreviews(type);
	}

	ResourcePreview GetResourcePreview(ResourceHandle const& handle)
	{
		return _ResourceManager->GetResourcePreview(handle);
	}

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

#endif // _D_EDITOR


	DResourceManager::DResourceManager()
	{
	}

	DResourceManager::~DResourceManager()
	{
		for (auto& typeClass : mResourceMap)
			for (auto& resourcePair : typeClass.second)
				resourcePair.second->Destroy();
	}

	Resource* DResourceManager::GetRawResource(D_CORE::Uuid const& uuid)
	{
		if (!mUuidMap.contains(uuid))
			throw D_EXCEPTION::Exception((std::string("Resource not found: uuid = ") + D_CORE::ToString(uuid)).c_str());
		return mUuidMap[uuid];
	}

	Resource* DResourceManager::GetRawResource(ResourceHandle handle)
	{
		if (handle.Type == 0)
			return nullptr;

		if (!mResourceMap.contains(handle.Type))
			throw D_EXCEPTION::Exception("Type not found");
		auto& typeClass = mResourceMap[handle.Type];
		if (!typeClass.contains(handle.Id))
			throw D_EXCEPTION::Exception("Resource with given id not found");
		return typeClass[handle.Id].get();
	}

	Resource* DResourceManager::GetRawResourceSafe(ResourceHandle handle)
	{
		if (handle.Type == 0)
			return nullptr;

		if (!mResourceMap.contains(handle.Type))
			return nullptr;
		auto& typeClass = mResourceMap[handle.Type];
		if (!typeClass.contains(handle.Id))
			return nullptr;
		return typeClass[handle.Id].get();
	}

	DVector<ResourcePreview> DResourceManager::GetResourcePreviews(ResourceType type)
	{
		DVector<ResourcePreview> res;
		for (auto& resource : mResourceMap.at(type))
		{
			res.push_back(*resource.second);
		}

		return res;
	}

	ResourcePreview DResourceManager::GetResourcePreview(ResourceHandle const& handle)
	{
		auto res = GetRawResourceSafe(handle);
		if (res == nullptr)
			return { L"", D_FILE::Path(), EmptyResourceHandle};

		return *res;
	}

	ResourceHandle DResourceManager::CreateResource(ResourceType type, Uuid const& uuid, std::wstring const& path, std::wstring const& name, bool isDefault, bool fromFile)

	{
		if (!fromFile && D_H_ENSURE_FILE(path))
			throw D_EXCEPTION::Exception(("A file with the same name already exists: " + WSTR2STR(path)).c_str());

		auto factory = Resource::GetFactoryForResourceType(type);
		if (!factory)
			return EmptyResourceHandle;

		auto res = factory->Create(uuid, path, name, GetNewId(), isDefault);

		res->mLoaded = !fromFile;
		res->mDirtyDisk = !fromFile;

		// Add the handle to path and resource maps
		ResourceHandle handle = { type , res->GetId() };
		UpdateMaps(res);

		return handle;
	}

	void DResourceManager::UpdateGPUResources()
	{
		for (auto& resType : mResourceMap)
		{
			for (auto& res : resType.second)
			{
				auto resource = res.second;
				if (resource->IsLoaded() && resource->IsDirtyGPU())
					resource->UpdateGPU();
			}
		}
	}

	void DResourceManager::UpdateMaps(std::shared_ptr<Resource> resource)
	{

		// Update resource map
		mResourceMap.at(resource->GetType()).try_emplace(resource->GetId(), resource);

		// Update uuid map
		mUuidMap.try_emplace(resource->GetUuid(), resource.get());

		// Update path map

		auto path = resource->GetPath().lexically_normal().wstring();

		auto& pathHandles = mPathMap[path];
		bool found = false;
		ResourceHandle newHandle = *resource;
		for (auto const& handle : pathHandles)
		{
			if (handle.Id == newHandle.Id && handle.Type == newHandle.Type)
			{
				found = true;
				break;
			}
		}
		if (!found)
			pathHandles.push_back(newHandle);
	}

	void DResourceManager::SaveAllResources()
	{
		for (auto& resType : mResourceMap)
		{
			for (auto& res : resType.second)
			{
				auto resource = res.second;
				if (resource->IsDirtyDisk())
					D_RESOURCE_LOADER::SaveResource(resource.get());
			}
		}
	}

#ifdef _D_EDITOR
	void DResourceManager::GetAllResources(DVector<Resource*>& resources) const
	{
		for (auto& resType : mResourceMap)
		{
			for (auto& res : resType.second)
			{
				resources.push_back(res.second.get());
			}
		}
	}

	void DResourceManager::GetAllResourcePaths(DVector<Path>& paths) const
	{
		for (auto const& path : mPathMap)
		{
			paths.push_back(Path(path.first));
		}
	}
#endif // _D_EDITOR
}