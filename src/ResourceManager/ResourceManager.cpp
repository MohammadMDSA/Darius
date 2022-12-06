#include "pch.hpp"
#include "ResourceManager.hpp"
#include "ResourceTypes/Resource.hpp"
#include "ResourceTypes/StaticMeshResource.hpp"
#include "ResourceTypes/SkeletalMeshResource.hpp"
#include "ResourceTypes/MaterialResource.hpp"
#include "ResourceTypes/BatchResource.hpp"
#include "ResourceTypes/TextureResource.hpp"
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

		_ResourceManager = std::make_unique<DResourceManager>();

		TextureResource::Register();
		StaticMeshResource::Register();
		SkeletalMeshResource::Register();
		MaterialResource::Register();
		BatchResource::Register();

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
			auto lowSphere = D_RENDERER_GEOMETRY_GENERATOR::CreateSphere(0.5f, 10, 6);
			auto line = D_RENDERER_GEOMETRY_GENERATOR::CreateLine(0.f, 0.f, 0.f, 0.f, 0.f, -1.f);

			auto resHandle = CreateResource<StaticMeshResource>(GenerateUuidFor("Box Mesh"), L"Box Mesh", L"Box Mesh", GetNewId(), true);
			MultiPartMeshData<StaticMeshResource::VertexType> meshData;
			meshData.meshParts = DVector<MeshData<StaticMeshResource::VertexType>>{ box };
			auto res = _GetRawResource(resHandle);
			((StaticMeshResource*)res)->Create(meshData);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::BoxMesh, { StaticMeshResource::GetResourceType(), res->GetId()} });

			resHandle = CreateResource<StaticMeshResource>(GenerateUuidFor("Cylinder Mesh"), L"Cylinder Mesh", L"Cylinder Mesh", GetNewId(), true);
			meshData.meshParts = DVector<MeshData<StaticMeshResource::VertexType>>{ cylinder };
			res = GetRawResource(resHandle);
			((StaticMeshResource*)res)->Create(meshData);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::CylinderMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<StaticMeshResource>(GenerateUuidFor("Geosphere Mesh"), L"Geosphere Mesh", L"Geosphere Mesh", GetNewId(), true);
			meshData.meshParts = DVector<MeshData<StaticMeshResource::VertexType>>{ geosphere };
			res = GetRawResource(resHandle);
			((StaticMeshResource*)res)->Create(meshData);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::GeosphereMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<StaticMeshResource>(GenerateUuidFor("Grid Mesh"), L"Grid Mesh", L"Grid Mesh", GetNewId(), true);
			meshData.meshParts = DVector<MeshData<StaticMeshResource::VertexType>>{ grid };
			res = GetRawResource(resHandle);
			((StaticMeshResource*)res)->Create(meshData);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::GridMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<StaticMeshResource>(GenerateUuidFor("Quad Mesh"), L"Quad Mesh", L"Quad Mesh", GetNewId(), true);
			meshData.meshParts = DVector<MeshData<StaticMeshResource::VertexType>>{ quad };
			res = GetRawResource(resHandle);
			((StaticMeshResource*)res)->Create(meshData);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::QuadMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<StaticMeshResource>(GenerateUuidFor("Sphere Mesh"), L"Sphere Mesh", L"Sphere Mesh", GetNewId(), true);
			meshData.meshParts = DVector<MeshData<StaticMeshResource::VertexType>>{ sphere };
			res = GetRawResource(resHandle);
			((StaticMeshResource*)res)->Create(meshData);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::SphereMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<StaticMeshResource>(GenerateUuidFor("Low Poly Sphere Mesh"), L"Low Poly Sphere Mesh", L"Low Poly Sphere Mesh", GetNewId(), true);
			meshData.meshParts = DVector<MeshData<StaticMeshResource::VertexType>>{ lowSphere };
			res = GetRawResource(resHandle);
			((StaticMeshResource*)res)->Create(meshData);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::LowPolySphereMesh, { StaticMeshResource::GetResourceType(), res->GetId() } });

			resHandle = CreateResource<BatchResource>(GenerateUuidFor("Line Mesh"), L"Line Mesh", L"Line Mesh", GetNewId(), true);
			meshData.meshParts = DVector<MeshData<StaticMeshResource::VertexType>>{ line };
			res = GetRawResource(resHandle);
			((BatchResource*)res)->Create(meshData);
			res->mDirtyGPU = false;
			res->mDirtyDisk = false;
			mDefaultResourceMap.insert({ DefaultResource::LineMesh, { BatchResource::GetResourceType(), res->GetId() } });
		}

		// Create default textures
		{
#define CreateDefaultTexture2D(name, color) \
{ \
	auto defaultTextureHandle = CreateResource<TextureResource>(GenerateUuidFor("Default Texture2D " #name), L"Default Texture2D " #name, L"Default Texture2D " #name, true, false); \
	auto textureRes = (TextureResource*)GetRawResource(defaultTextureHandle); \
	textureRes->CreateRaw(color, DXGI_FORMAT_R8G8B8A8_UNORM, 4, 1, 1); \
	auto rRes = dynamic_cast<Resource*>(textureRes); \
	rRes->mDirtyGPU = false; \
	rRes->mDirtyDisk = false; \
	mDefaultResourceMap.insert({ DefaultResource::Texture2D##name, { TextureResource::GetResourceType(), textureRes->GetId() } }); \
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
			auto defaultMaterialHandle = CreateResource<MaterialResource>(GenerateUuidFor("Default Material"), L"Default Material", L"Default Material", true, false);
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
		auto handle = CreateResource<MaterialResource>(GenerateUuid(), path, D_FILE::GetFileName(path), false, false);
		auto res = GetRawResource(handle);
		D_RESOURCE_LOADER::SaveResource(res);
		return handle;
	}

	ResourceHandle DResourceManager::CreateResource(ResourceType type, Uuid uuid, std::wstring const& path, std::wstring const& name, bool isDefault, bool fromFile)

	{
		if (!fromFile && D_H_ENSURE_FILE(path))
			throw D_EXCEPTION::Exception(("A file with the same name already exists: " + STR_WSTR(path)).c_str());

		auto factory = Resource::GetFactoryForResourceType(type);
		if (!factory)
			return EmptyResourceHandle;

		auto res = factory->Create(uuid, path, name, GetNewId(), isDefault);

		res->mLoaded = !fromFile;

		// Add the handle to path and resource maps
		ResourceHandle handle = { type , res->GetId() };
		UpdateMaps(res);

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
		auto& pathHandles = mPathMap[resource->GetPath().wstring()];
		bool found = false;
		ResourceHandle newHandle = *resource;
		for (auto const& handle : pathHandles)
		{
			if (handle.Id == newHandle.Id && handle.Type == newHandle.Type)
			{
				found = true;
				break;
			}
		}
		if (!found)
			pathHandles.push_back(newHandle);
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

#ifdef _D_EDITOR
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
#endif // _D_EDITOR
}