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

	D_CORE::StringIdDatabase Resource::NameDatabase;

	DUnorderedMap<ResourceType, D_CORE::StringId> Resource::ResourceTypeMap =
	{
		{ 0, ""_Res }
	};

	DUnorderedMap<D_CORE::StringId, ResourceType> Resource::ResourceTypeMapR =
	{
		{ ""_Res, 0}
	};

	DUnorderedMap<ResourceType, Resource::ResourceFactory*> Resource::ResourceFactories =
	{
		{ 0, nullptr }
	};

	DUnorderedMap<std::string, ResourceType> Resource::ResourceExtensionMap = {};

	DUnorderedMap<ResourceType, std::function<SubResourceConstructionData(ResourceType type, Path const&)>> Resource::ConstructValidationMap = {};

	D_CORE::StringId ResourceTypeToString(ResourceType type)
	{
		auto name = Resource::GetResourceName(type);
		if(name == ""_Id)
			throw D_CORE::Exception::Exception("Resource type not defined");
		return name;
	}

	ResourceType StringToResourceType(D_CORE::StringId name)
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

	SubResourceConstructionData Resource::CanConstructFrom(ResourceType type, Path const& path)
	{
		ResourceDataInFile data;
		auto name = D_FILE::GetFileName(path);
		data.Name = WSTR2STR(name);
		data.Type = type;
		return SubResourceConstructionData {.SubResources = { data }};
	}

	bool Resource::Release()
	{
		//Unload();
		//mLoaded.store(false);
		return true;
	}
}
