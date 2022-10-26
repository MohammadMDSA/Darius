#include "Renderer/pch.hpp"
#include "Resource.hpp"

#include "ResourceManager/ResourceManager.hpp"

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

	DUnorderedMap<ResourceType, std::function<bool(Path const&)>> Resource::ConstructValidationMap = {};

	std::string ResourceTypeToString(ResourceType type)
	{
		auto name = Resource::GetResourceName(type);
		if(name == "")
			throw D_CORE::Exception::Exception("Resource type not defined");
		return name;
	}

	ResourceType StringToResourceType(std::string name)
	{
		return Resource::GetResourceType(name);
	}

	void Resource::UpdateGPU(D_GRAPHICS::GraphicsContext& context)
	{
		// Is gpu already up to date
		if (!mDirtyGPU)
			return;

		mDirtyGPU = !UploadToGpu(context);
	}

	void Resource::AddTypeContainer(ResourceType type)
	{
		D_RESOURCE::GetManager()->mResourceMap[type];
	}

}
