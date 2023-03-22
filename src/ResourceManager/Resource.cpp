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

	void Resource::UpdateGPU()
	{
		// Is gpu already up to date
		if (!mDirtyGPU)
			return;

		mDirtyGPU = !UploadToGpu();
	}

	void Resource::AddTypeContainer(ResourceType type)
	{
		D_RESOURCE::GetManager()->mResourceMap[type];
	}

	DVector<ResourceDataInFile> Resource::CanConstructFrom(ResourceType type, Path const& path)
	{
		ResourceDataInFile data;
		auto name = D_FILE::GetFileName(path);
		data.Name = STR_WSTR(name);
		data.Type = type;
		return { data };
	}

}
