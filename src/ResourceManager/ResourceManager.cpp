#include "pch.hpp"
#include "ResourceManager.hpp"
#include "ResourceTypes/Resource.hpp"
#include "ResourceTypes/MeshResource.hpp"
#include "ResourceTypes/MaterialResource.hpp"
#include "ResourceTypes/BatchResource.hpp"
#include "ResourceTypes/Texture2DResource.hpp"
#include "ResourceLoader.hpp"

#include <Core/Filesystem/Path.hpp>
#include <Core/Filesystem/FileUtils.hpp>
#include <Core/Containers/Map.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Renderer/Geometry/GeometryGenerator.hpp>
#include <Utils/Assert.hpp>

using namespace D_CONTAINERS;

namespace Darius::ResourceManager
{
	bool _initialized = false;

	std::unique_ptr<DResourceManager>				_ResourceManager;

	void Initialize()
	{
		D_ASSERT(_ResourceManager == nullptr);

		Texture2DResource::Register();
		MeshResource::Register();
		MaterialResource::Register();
		BatchResource::Register();

		_ResourceManager = std::make_unique<DResourceManager>();
		_ResourceManager->LoadDefaultResources();
	}

	void Shutdown()
	{
		D_ASSERT(_ResourceManager);
	}

	DResourceManager* GetManager()
	{
		return _ResourceManager.get();
	}

	Resource* _GetRawResource(Uuid uuid, bool load)
	{
		auto resource = _ResourceManager->GetRawResource(uuid);

		// Load resource if not loaded yet
		if (load && !resource->GetLoaded())
			D_RESOURCE_LOADER::LoadResource(resource);
		if (load && resource->GetDirtyGPU())
		{
			auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Resource uploader");
			resource->UpdateGPU(context);
			context.Finish(true);
		}
		return resource;
	}

	Resource* _GetRawResource(ResourceHandle handle, bool load)
	{
		auto resource = _ResourceManager->GetRawResource(handle);

		// Load resource if not loaded yet
		if (load && !resource->GetLoaded())
			D_RESOURCE_LOADER::LoadResource(resource);
		if (load && resource->GetDirtyGPU())
		{
			auto& context = D_GRAPHICS::GraphicsContext::Begin(L"Resource uploader");
			resource->UpdateGPU(context);
			context.Finish(true);
		}
		return resource;
	}

	ResourceHandle GetResourceHandle(Uuid uuid)
	{
		return *_ResourceManager->GetRawResource(uuid);
	}

	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type)
	{
		return _ResourceManager->GetResourcePreviews(type);
	}

	ResourceHandle GetDefaultResource(DefaultResource type)
	{
		return _ResourceManager->GetDefaultResource(type);
	}

	void UpdateGPUResources(D_GRAPHICS::GraphicsContext& context)
	{
		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, "Update GPU Resources");
		_ResourceManager->UpdateGPUResources(context);
		PIXEndEvent(context.GetCommandList());
	}

	void SaveAll()
	{
		_ResourceManager->SaveAllResources();
	}

#ifdef _D_EDITOR
	void GetAllResources(DVector<Resource*>& resources)
	{
		_ResourceManager->GetAllResources(resources);
	}
#endif // _D_EDITOR


	DResourceManager::DResourceManager()
	{
		for (auto [resType, _] : Resource::ResourceTypeMap)
		{
			mResourceMap[resType];
		}

	}

	DResourceManager::~DResourceManager()
	{
		for (auto& typeClass : mResourceMap)
			for (auto& resourcePair : typeClass.second)
				resourcePair.second->Destroy();
	}

	Resource* DResourceManager::GetRawResource(Uuid uuid)
	{
		if (!mUuidMap.contains(uuid))
			throw D_EXCEPTION::Exception((std::string("Resource not found: uuid = ") + D_CORE::ToString(uuid)).c_str());
		return mUuidMap[uuid];
	}

	Resource* DResourceManager::GetRawResource(ResourceHandle handle)
	{
		if (handle.Type == 0)
			return nullptr;

		if (!mResourceMap.contains(handle.Type))
			throw D_EXCEPTION::Exception("Type not found");
		auto typeClass = mResourceMap[handle.Type];
		if (!typeClass.contains(handle.Id))
			throw D_EXCEPTION::Exception("Resource with given id not found");
		return typeClass[handle.Id].get();
	}

	DVector<ResourcePreview> DResourceManager::GetResourcePreviews(ResourceType type)
	{
		DVector<ResourcePreview> res;
		for (auto& resource : mResourceMap.at(type))
		{
			res.push_back(*resource.second);
		}

		return res;
	}

	void DResourceManager::LoadDefaultResources()
	{
		// Creating default meshes
		{
			auto box = D_RENDERER_GEOMETRY_GENERATOR::CreateBox(1.f, 1.f, 1.f, 0);
			auto cylinder = D_RENDERER_GEOMETRY_GENERATOR::CreateCylinder(0.5f, 0.5f, 1, 40, 20);
			auto geosphere = D_RENDERER_GEOMETRY_GENERATOR::CreateGeosphere(0.5f, 40);
			auto grid = D_RENDERER_GEOMETRY_GENERATOR::CreateGrid(100.f, 100.f, 100, 100);
			auto quad = D_RENDERER_GEOMETRY_GENERATOR::CreateQuad(0.f, 0.f, 1.f, 1.f, 0.f);
			auto sphere = D_RENDERER_GEOMETRY_GENERATOR::CreateSphere(0.5f, 40, 40);
			auto line = D_RENDERER_GEOMETRY_GENERATOR::CreateLine(0.f, 0.f, 0.f, 0.f, 0.f, -1.f);

			auto resHandle = CreateResource<MeshResource>(GenerateUuidFor("Box Mesh"), L"Box Mesh", GetNewId(), true);
			auto meshVec = DVector<MeshData<MeshResource::VertexType>>{ box };
			auto res = _GetRawResource(resHandle);
			((MeshResource*)res)->Create(meshVec);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::BoxMesh, { MeshResource::GetResourceType(), res->GetId()} });

			resHandle = CreateResource<MeshResource>(GenerateUuidFor("Cylinder Mesh"), L"Cylinder Mesh", GetNewId(), true);
			meshVec = DVector<MeshData<MeshResource::VertexType>>{ cylinder };
			res = GetRawResource(resHandle);
			((MeshResource*)res)->Create(meshVec);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::CylinderMesh, { MeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<MeshResource>(GenerateUuidFor("Geosphere Mesh"), L"Geosphere Mesh", GetNewId(), true);
			meshVec = DVector<MeshData<MeshResource::VertexType>>{ geosphere };
			res = GetRawResource(resHandle);
			((MeshResource*)res)->Create(meshVec);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::GeosphereMesh, { MeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<MeshResource>(GenerateUuidFor("Grid Mesh"), L"Grid Mesh", GetNewId(), true);
			meshVec = DVector<MeshData<MeshResource::VertexType>>{ grid };
			res = GetRawResource(resHandle);
			((MeshResource*)res)->Create(meshVec);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::GridMesh, { MeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<MeshResource>(GenerateUuidFor("Quad Mesh"), L"Quad Mesh", GetNewId(), true);
			meshVec = DVector<MeshData<MeshResource::VertexType>>{ quad };
			res = GetRawResource(resHandle);
			((MeshResource*)res)->Create(meshVec);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::QuadMesh, { MeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<MeshResource>(GenerateUuidFor("Sphere Mesh"), L"Sphere Mesh", GetNewId(), true);
			meshVec = DVector<MeshData<MeshResource::VertexType>>{ sphere };
			res = GetRawResource(resHandle);
			((MeshResource*)res)->Create(meshVec);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::SphereMesh, { MeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<BatchResource>(GenerateUuidFor("Line Mesh"), L"Line Mesh", GetNewId(), true);
			meshVec = DVector<MeshData<MeshResource::VertexType>>{ line };
			res = GetRawResource(resHandle);
			((BatchResource*)res)->Create(meshVec);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::LineMesh, { BatchResource::GetResourceType(), res->GetId() } });
		}

		// Create default textures
		{
#define CreateDefaultTexture2D(name, color) \
{ \
	auto defaultTextureHandle = CreateResource<Texture2DResource>(GenerateUuidFor("Default Texture2D " #name), L"Default Texture2D " #name, true, false); \
	auto textureRes = (Texture2DResource*)GetRawResource(defaultTextureHandle); \
	textureRes->CreateRaw(color, DXGI_FORMAT_R8G8B8A8_UNORM, 4, 1, 1); \
	auto rRes = dynamic_cast<Resource*>(textureRes); \
	rRes->mDirtyGPU = false; \
	rRes->mDirtyDisk = false; \
	mDefaultResourceMap.insert({ DefaultResource::Texture2D##name, { Texture2DResource::GetResourceType(), textureRes->GetId() } }); \
}

			CreateDefaultTexture2D(Magenta, 0xFFFF00FF);
			CreateDefaultTexture2D(BlackOpaque, 0xFF000000);
			CreateDefaultTexture2D(BlackTransparent, 0x00000000);
			CreateDefaultTexture2D(WhiteOpaque, 0xFFFFFFFF);
			CreateDefaultTexture2D(WhiteTransparent, 0x00FFFFFF);
			CreateDefaultTexture2D(NormalMap, 0x00FF8080);

		}

		// Creating default materials
		{
			auto defaultMaterialHandle = CreateResource<MaterialResource>(GenerateUuidFor("Default Material"), L"Default Material", true, false);
			auto materialRes = (MaterialResource*)GetRawResource(defaultMaterialHandle);
			auto mat = materialRes->ModifyMaterialData();
			mat->DifuseAlbedo = XMFLOAT4(Vector4(kOne));
			mat->FresnelR0 = XMFLOAT3(Vector3(kZero));
			mat->Roughness = 0.2f;
			auto rRes = dynamic_cast<Resource*>(materialRes);
			rRes->mDirtyGPU = true;
			rRes->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::Material, { MaterialResource::GetResourceType(), materialRes->GetId() } });
		}

	}

	ResourceHandle DResourceManager::CreateMaterial(std::wstring const& dirpath)
	{

		if (!D_H_ENSURE_DIR(dirpath))
			throw D_EXCEPTION::FileNotFoundException("Specified directory not found: " + STR_WSTR(dirpath));
		auto parent = Path(dirpath);

		auto path = parent.append(D_FILE::GetNewFileName(L"New Material", L".mat", parent));

		// Create resource
		auto handle = CreateResource<MaterialResource>(GenerateUuid(), path, false, false);
		auto res = GetRawResource(handle);
		D_RESOURCE_LOADER::SaveResource(res);
		return handle;
	}

	ResourceHandle DResourceManager::GetDefaultResource(DefaultResource type)
	{
		return mDefaultResourceMap.at(type);
	}

	void DResourceManager::UpdateGPUResources(D_GRAPHICS::GraphicsContext& context)
	{
		for (auto& resType : mResourceMap)
		{
			for (auto& res : resType.second)
			{
				auto resource = res.second;
				if (resource->GetDirtyGPU() && resource->GetLoaded())
					resource->UpdateGPU(context);
			}
		}
	}

	void DResourceManager::UpdateMaps(std::shared_ptr<Resource> resource)
	{

		// Update resource map
		mResourceMap.at(resource->GetType()).try_emplace(resource->GetId(), resource);

		// Update uuid map
		mUuidMap.try_emplace(resource->GetUuid(), resource.get());

		// Update path map
		mPathMap.try_emplace(resource->GetPath().wstring(), resource.get());
	}

	void DResourceManager::SaveAllResources()
	{
		for (auto& resType : mResourceMap)
		{
			for (auto& res : resType.second)
			{
				auto resource = res.second;
				if (resource->GetDirtyDisk())
					D_RESOURCE_LOADER::SaveResource(resource.get());
			}
		}
	}

	void DResourceManager::GetAllResources(DVector<Resource*>& resources)
	{
		for (auto& resType : mResourceMap)
		{
			for (auto& res : resType.second)
			{
				resources.push_back(res.second.get());
			}
		}
	}
}