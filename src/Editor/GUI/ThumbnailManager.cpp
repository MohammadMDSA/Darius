#include "Editor/pch.hpp"
#include "ThumbnailManager.hpp"

#include "Editor/EditorContext.hpp"
#include "GuiRenderer.hpp"

#include <Core/Containers/Map.hpp>
#include <Renderer/Resources/MaterialResource.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
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
		auto textureHandle = D_GUI_RENDERER::AllocateUiTexture(); \
		ResourceTypeTextureIdMap[resourceType] = textureHandle.GetGpuPtr(); \
		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptorsSimple(1, textureHandle, texture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); \
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
	DUnorderedMap<Uuid, D_GRAPHICS_BUFFERS::Texture, UuidHasher> ResourceTextures;
	DUnorderedMap<CommonIcon, D_GRAPHICS_BUFFERS::Texture>		CommonIconTextures;

	DUnorderedMap<ResourceType, uint64_t>						ResourceTypeTextureIdMap;
	DUnorderedMap<Uuid, uint64_t, UuidHasher>					ResourceTextureIdMap;
	DUnorderedMap<CommonIcon, uint64_t>							CommonIconTextureIdMap;

	////////////////////////////////////////////////////////////////
	////// Options
	std::string													ThumbnailCacheDirectory = "CacheData/Thumbnails";

	void RegisterResourceTypeTextures();
	void RegisterCommonIconTextures();
	void AddExistingResourceThumbnail(Uuid const& uuid); // Add thumbnail of previously generated thumbnail to registry by uuid
	void GenerateThumbnailForResource(Resource const* resource, bool force = false);
	D_FILE::Path GetThumbnailsDirectory();

	void GenerateThumbnailFromDDSTexture(Uuid const& resourceUuid, D_FILE::Path const& path);

	void Initialize()
	{
		D_ASSERT(!_initialized);

		RegisterCommonIconTextures();
		RegisterResourceTypeTextures();

		// Adding existing thumbnails
		auto thumbnailDir = GetThumbnailsDirectory();
		if (D_H_ENSURE_DIR(thumbnailDir))
			D_FILE::VisitFilesInDirectory(thumbnailDir, true, [&](auto const& path)
				{
					auto uuid = FromWString(D_FILE::GetFileName(path));
					AddExistingResourceThumbnail(uuid);
				});

		_initialized = true;
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

	D_FILE::Path GetThumbnailsDirectory()
	{
		return D_EDITOR_CONTEXT::GetProjectPath() / ThumbnailCacheDirectory;
	}

	D_FILE::Path GetResourceThumbnailPath(Uuid uuid)
	{
		return GetThumbnailsDirectory() / (D_CORE::ToString(uuid) + ".dds");
	}

	void RegisterResourceTypeTextures()
	{
		REGISTER_RESOURCE_TYPE_TEXTURE(D_RENDERER::MaterialResource, "material.dds");

	}

	void RegisterExistingResources(D_FILE::Path const& path)
	{

		static ResourceType supportedTypes[] = { D_RENDERER::TextureResource::GetResourceType() };

		for (int i = 0; i < _countof(supportedTypes); i++)
		{
			auto resourcesOfType = D_RESOURCE::GetResourcePreviews(supportedTypes[i]);
			for (auto resourcePrev : resourcesOfType)
			{
				auto resource = D_RESOURCE::GetRawResourceSync(resourcePrev.Handle, false);
				auto resourcePath = resource->GetPath();
				if (!resource->IsDefault() && resourcePath.has_parent_path() && std::filesystem::equivalent(resourcePath.parent_path(), path))
					GenerateThumbnailForResource(resource);
			}
		}
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
			auto& tex = CommonIconTextures[(CommonIcon)i];

			auto fileData = D_FILE::ReadFileSync(resPath / "icons" / iconFiles[i]);

			tex.CreateDDSFromMemory(fileData->data(), fileData->size(), true);

			auto gpuHandle = D_GUI_RENDERER::AllocateUiTexture(1);

			D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptorsSimple(1, gpuHandle, tex.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			CommonIconTextureIdMap[(CommonIcon)i] = gpuHandle.GetGpuPtr();
		}
	}

	uint64_t GetIconTextureId(CommonIcon iconId)
	{
		return CommonIconTextureIdMap[iconId];
	}

	uint64_t GetResourceTextureId(D_RESOURCE::ResourceHandle const& handle)
	{
		auto resource = D_RESOURCE::GetRawResourceSync(handle, false);
		auto uuid = resource->GetUuid();
		if (ResourceTextureIdMap.contains(uuid))
			return ResourceTextureIdMap[uuid];

		auto type = resource->GetType();
		if (ResourceTypeTextureIdMap.contains(type))
			return ResourceTypeTextureIdMap[type];

		return CommonIconTextureIdMap[CommonIcon::File];
	}

	void AddExistingResourceThumbnail(Uuid const& uuid)
	{

		// Return if texture already 
		if (ResourceTextures.contains(uuid))
			return;

		auto& tex = ResourceTextures[uuid];

		auto fileData = D_FILE::ReadFileSync(GetResourceThumbnailPath(uuid));

		tex.CreateDDSFromMemory(fileData->data(), fileData->size(), true);

		// Don't create texture if not
		if (!tex.Is2D() || tex.ArraySize() > 1)
		{
			ResourceTextures.erase(uuid);
			return;
		}

		auto gpuHandle = D_GUI_RENDERER::AllocateUiTexture(1);

		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptorsSimple(1, gpuHandle, tex.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		ResourceTextureIdMap[uuid] = gpuHandle.GetGpuPtr();
	}

	void GenerateThumbnailForResource(Resource const* resource, bool force)
	{
		auto resourceType = resource->GetType();
		auto uuid = resource->GetUuid();

		// Return if texture already 
		if (!force && ResourceTextures.contains(uuid))
			return;

		if (resourceType == D_RENDERER::TextureResource::GetResourceType())
		{
			GenerateThumbnailFromDDSTexture(uuid, resource->GetPath());
		}
		else
			return;

		AddExistingResourceThumbnail(uuid);
	}

	void GenerateThumbnailFromDDSTexture(Uuid const& resourceUuid, D_FILE::Path const& path)
	{

		auto destinationFolder = GetThumbnailsDirectory();
		auto finalDestinationFile = (destinationFolder / (D_CORE::ToString(resourceUuid) + ".dds")).lexically_normal();

		if (!D_H_ENSURE_DIR(destinationFolder))
			std::filesystem::create_directories(destinationFolder);

		auto normalPath = path.lexically_normal();

		std::string outputExt = normalPath.extension().string();
		outputExt = outputExt.substr(1, outputExt.size() - 1);
		boost::algorithm::to_lower(outputExt);
		auto thumbnaildCommand = std::string("Utils\\texconv.exe -m 1 -w 64 -h 64 -o \"" + destinationFolder.string() + "\" \"" + normalPath.string() + "\"");

		system(thumbnaildCommand.c_str());

		std::filesystem::rename(destinationFolder / normalPath.filename().replace_extension(".dds"), finalDestinationFile);
	}
}
