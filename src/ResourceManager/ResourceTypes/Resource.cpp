#include "Renderer/pch.hpp"
#include "Resource.hpp"

namespace Darius::ResourceManager
{

	DMap<ResourceType, std::string> Resource::ResourceTypeMap =
	{
		{ 0, "" }
	};

	DMap<std::string, ResourceType> Resource::ResourceTypeMapR =
	{
		{ "", 0 }
	};

	DMap<ResourceType, Resource::ResourceFactory*> Resource::ResourceFactories =
	{
		{ 0, nullptr }
	};

	DMap<std::string, ResourceType> Resource::ResourceExtensionMap = {};

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

}
