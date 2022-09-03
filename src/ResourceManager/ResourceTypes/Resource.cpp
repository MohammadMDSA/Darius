#include "Renderer/pch.hpp"
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
		case Darius::ResourceManager::ResourceType::Batch:
			return "Batch";
		case Darius::ResourceManager::ResourceType::Texture2D:
			return "Texture2D";
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
		if (type == "Batch")
			return ResourceType::Batch;
		if (type == "Texture2D")
			return ResourceType::Texture2D;
		return ResourceType::None;
	}

	void Resource::UpdateGPU(D_GRAPHICS::GraphicsContext& context)
	{
		// Is gpu already up to date
		if (!mDirtyGPU)
			return;

		mDirtyGPU = UploadToGpu(context);
	}

}
