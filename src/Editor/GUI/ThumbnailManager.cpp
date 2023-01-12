#include "Editor/pch.hpp"
#include "ThumbnailManager.hpp"

#include "Editor/EditorContext.hpp"

#include <Core/Containers/Map.hpp>
#include <Renderer/Resources/MaterialResource.hpp>

#include <Utils/Assert.hpp>

#define REGISTER_RESOURCE_TYPE_TEXTURE(resType, fileName) \
{ \
auto resPath = D_FILE::Path("EditorResources") / "icons" / fileName; \
auto textureFile = D_FILE::ReadFileSync(resPath.wstring()); \
if (textureFile->size() > 0) \
{ \
	auto resourceType = resType::GetResourceType(); \
	auto& texture = ResourceTypeTextures[resourceType]; \
	if (texture.CreateDDSFromMemory(textureFile->data(), textureFile->size(), true)) \
	{ \
		auto textureHandle = D_RENDERER::AllocateUiTexture(); \
		ResourceTypeTextureIdMap[resourceType] = textureHandle.GetGpuPtr(); \
		D_RENDERER_DEVICE::GetDevice()->CopyDescriptorsSimple(1, textureHandle, texture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); \
	} \
	else \
		ResourceTypeTextures.erase(resourceType); \
} \
}

using namespace D_CONTAINERS;
using namespace D_RESOURCE;
using namespace D_CORE;

namespace Darius::Editor::Gui::ThumbnailManager
{
	bool								_initialized = false;
	DUnorderedMap<ResourceType, D_GRAPHICS_BUFFERS::Texture>	ResourceTypeTextures;
	DUnorderedMap<Uuid, D_GRAPHICS_BUFFERS::Texture>			ResourceTextures;
	DUnorderedMap<CommonIcon, D_GRAPHICS_BUFFERS::Texture>		CommonIconTextures;

	DUnorderedMap<ResourceType, uint64_t>						ResourceTypeTextureIdMap;
	DUnorderedMap<Uuid, uint64_t>								ResourceTextureIdMap;
	DUnorderedMap<CommonIcon, uint64_t>							CommonIconTextureIdMap;

	void RegisterResourceTypeTextures();
	void RegisterCommonIconTextures();

	void Initialize()
	{
		D_ASSERT(!_initialized);

		RegisterCommonIconTextures();
		RegisterResourceTypeTextures();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		ResourceTypeTextures.clear();
		ResourceTextures.clear();
		CommonIconTextures.clear();

		ResourceTypeTextureIdMap.clear();
		ResourceTextureIdMap.clear();
		CommonIconTextureIdMap.clear();
	}

	void RegisterResourceTypeTextures()
	{
		REGISTER_RESOURCE_TYPE_TEXTURE(D_GRAPHICS::MaterialResource, "material.dds");

	}

	void RegisterCommonIconTextures()
	{
		std::string iconFiles[] =
		{
			"folder.dds",
			"document.dds"
		};

		auto resPath = D_FILE::Path("EditorResources");

		for (UINT i = 0; i < (UINT)CommonIcon::NumIcons; i++)
		{
			CommonIconTextures[(CommonIcon)i] = D_GRAPHICS_BUFFERS::Texture();
			auto& tex = CommonIconTextures[(CommonIcon)i];

			auto fileData = D_FILE::ReadFileSync(resPath / "icons" / iconFiles[i]);

			tex.CreateDDSFromMemory(fileData->data(), fileData->size(), true);

			auto gpuHandle = D_RENDERER::AllocateUiTexture(1);

			D_RENDERER_DEVICE::GetDevice()->CopyDescriptorsSimple(1, gpuHandle, tex.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CommonIconTextureIdMap[(CommonIcon)i] = gpuHandle.GetGpuPtr();
		}
	}

	uint64_t GetIconTextureId(CommonIcon iconId)
	{
		return CommonIconTextureIdMap[iconId];
	}

	uint64_t GetResourceTextureId(D_RESOURCE::ResourceHandle const& handle)
	{
		auto resource = D_RESOURCE::_GetRawResource(handle, false);
		auto uuid = resource->GetUuid();
		if (ResourceTextureIdMap.contains(uuid))
			return ResourceTextureIdMap[uuid];

		auto type = resource->GetType();
		if (ResourceTypeTextureIdMap.contains(type))
			return ResourceTypeTextureIdMap[type];

		return CommonIconTextureIdMap[CommonIcon::File];
	}

}
