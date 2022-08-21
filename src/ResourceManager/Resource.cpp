#include "pch.hpp"
#include "Resource.hpp"

namespace Darius::ResourceManager
{

	std::string ResourceTypeToString(ResourceType type)
	{
		switch (type)
		{
		case Darius::ResourceManager::ResourceType::Mesh:
			return "Mesh";
		case Darius::ResourceManager::ResourceType::Material:
			return "Material";
		default:
			throw D_CORE::Exception::Exception("Resource type not defined");
		case Darius::ResourceManager::ResourceType::None:
			return "";
		}
	}

	ResourceType StringToResourceType(std::string type)
	{
		if (type == "Mesh")
			return ResourceType::Mesh;
		if (type == "Material")
			return ResourceType::Material;
		return ResourceType::None;
	}

	void Resource::UpdateGPU(D_GRAPHICS::GraphicsContext& context)
	{
		// Is gpu already up to date
		if (!mDirtyGPU)
			return;

		UploadToGpu(context);
		mDirtyGPU = false;
	}

}
