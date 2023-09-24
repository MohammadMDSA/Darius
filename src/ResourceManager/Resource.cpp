#include "pch.hpp"
#include "Resource.hpp"

#include "ResourceManager/ResourceManager.hpp"

#include "Resource.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_FILE;

namespace Darius::ResourceManager
{

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

	DUnorderedMap<std::string, D_CONTAINERS::DSet<ResourceType>> Resource::ResourceExtensionMap = {};

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

	bool Resource::UpdateGPU()
	{
		// Update dependencies first
		if (AreDependenciesDirty())
		{
			// If dependencies are dirty, I mark myself to be updated when they are done
			MakeGpuDirty();
			return false;
		}

		// Is gpu already up to date
		if (!IsSelfDirtyGPU())
			return false;

		SetLocked(true);

		auto result = UploadToGpu();

		SetLocked(false);

		return result;
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
		Unload();
		mLoaded = false;
		return true;
	}
}
