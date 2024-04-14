#include "pch.hpp"
#include "Resource.hpp"

#include "ResourceManager/ResourceManager.hpp"

#include "Resource.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_FILE;

namespace Darius::ResourceManager
{

#if _D_EDITOR
	D_CORE::Signal<void(D_FILE::Path const&, Darius::ResourceManager::ResourceHandle const&, bool selected)> Resource::RequestPathChange;
#endif // _D_EDITOR

	DUnorderedMap<ResourceType, std::string> Resource::ResourceTypeMap =
	{
		{ 0, "" }
	};

	DUnorderedMap<std::string, ResourceType> Resource::ResourceTypeMapR =
	{
		{ "", 0 }
	};

	DUnorderedMap<ResourceType, Resource::ResourceFactory*> Resource::ResourceFactories =
	{
		{ 0, nullptr }
	};

	DUnorderedMap<std::string, ResourceType> Resource::ResourceExtensionMap = {};

	DUnorderedMap<ResourceType, std::function<DVector<ResourceDataInFile>(ResourceType type, Path const&)>> Resource::ConstructValidationMap = {};

	std::string ResourceTypeToString(ResourceType type)
	{
		auto name = Resource::GetResourceName(type);
		if(name == "")
			throw D_CORE::Exception::Exception("Resource type not defined");
		return name;
	}

	ResourceType StringToResourceType(std::string name)
	{
		return Resource::GetResourceTypeFromName(name);
	}

	ResourceGpuUpdateResult Resource::UpdateGPU()
	{
		// Update dependencies first
		if (AreDependenciesDirty())
		{
			// If dependencies are dirty, I mark myself to be updated when they are done
			MakeGpuDirty();
			return ResourceGpuUpdateResult::DirtyDependency;
		}

		// Is gpu already up to date
		if (mDirtyGPU.load() != GPUDirtyState::Dirty)
			return ResourceGpuUpdateResult::AlreadyClean;

		SetLocked(true);

		auto result = UploadToGpu();

		SetLocked(false);

		return ResourceGpuUpdateResult::Success;
	}

	void Resource::AddTypeContainer(ResourceType type)
	{
		D_RESOURCE::GetManager()->mResourceMap[type];
	}

	void Resource::SignalChange()
	{
		OnChange();

		mChangeSignal(this);
	}

	DVector<ResourceDataInFile> Resource::CanConstructFrom(ResourceType type, Path const& path)
	{
		ResourceDataInFile data;
		auto name = D_FILE::GetFileName(path);
		data.Name = WSTR2STR(name);
		data.Type = type;
		return { data };
	}

	bool Resource::Release()
	{
		//Unload();
		//mLoaded.store(false);
		return true;
	}
}
