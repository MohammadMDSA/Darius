#include "pch.hpp"
#include "ResourceManager.hpp"
#include "Resource.hpp"
#include "MeshResource.hpp"
#include "MaterialResource.hpp"
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
	const DMap<std::string, ResourceType>			ResourceTypeMap =
	{
		{ MeshResource::GetTypeName(), ResourceType::Mesh },
		{ MaterialResource::GetTypeName(), ResourceType::Material }
	};

	void Initialize()
	{
		D_ASSERT(_ResourceManager == nullptr);
		_ResourceManager = std::make_unique<DResourceManager>();
	}

	void Shutdown()
	{
		D_ASSERT(_ResourceManager);
	}

	ResourceHandle CreateResource(std::wstring path, ResourceType type)
	{
		switch (type)
		{
		case Darius::ResourceManager::ResourceType::None:
		case Darius::ResourceManager::ResourceType::Mesh:
		default:
			return { ResourceType::None, 0 };
		case Darius::ResourceManager::ResourceType::Material:
			return _ResourceManager->CreateMaterial(path);
		}
	}

	DResourceManager* GetManager()
	{
		return _ResourceManager.get();
	}

	Resource* _GetRawResource(ResourceHandle handle, bool load)
	{
		auto resource = _ResourceManager->GetRawResource(handle);

		// Load resouce if not loaded yet
		if (load && !resource->GetLoaded())
			D_RESOURCE_LOADER::LoadResource(resource);
		return resource;
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
		mResourceMap.insert({ ResourceType::Mesh, DMap<DResourceId, Resource*>() });
		mResourceMap.insert({ ResourceType::Material, DMap<DResourceId, Resource*>() });

		LoadDefaultResources();
	}

	DResourceManager::~DResourceManager()
	{
		for (auto& typeClass : mResourceMap)
			for (auto& resourcePair : typeClass.second)
				resourcePair.second->Destroy();
	}

	Resource* DResourceManager::GetRawResource(ResourceHandle handle)
	{
		if (handle.Type == ResourceType::None)
			return nullptr;

		if (!mResourceMap.contains(handle.Type))
			throw D_EXCEPTION::Exception("Type not found");
		auto typeClass = mResourceMap[handle.Type];
		if (!typeClass.contains(handle.Id))
			throw D_EXCEPTION::Exception("Resource with given id not found");
		return typeClass[handle.Id];
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
		auto box = D_RENDERER_GEOMETRY_GENERATOR::CreateBox(1.f, 1.f, 1.f, 1);
		auto cylinder = D_RENDERER_GEOMETRY_GENERATOR::CreateCylinder(0.5f, 0.5f, 1, 40, 20);
		auto geosphere = D_RENDERER_GEOMETRY_GENERATOR::CreateGeosphere(0.5f, 40);
		auto grid = D_RENDERER_GEOMETRY_GENERATOR::CreateGrid(100.f, 100.f, 100, 100);
		auto quad = D_RENDERER_GEOMETRY_GENERATOR::CreateQuad(0.f, 0.f, 1.f, 1.f, 0.f);
		auto sphere = D_RENDERER_GEOMETRY_GENERATOR::CreateSphere(0.5f, 40, 40);
		auto line = D_RENDERER_GEOMETRY_GENERATOR::CreateLine(0.f, 0.f, 0.f, 0.f, 0.f, -1.f);

		// TODO: bad allocation
		auto res = new MeshResource(GenerateUuidFor("Box Mesh"), L"Box Mesh", GetNewId(), true);
		res->Create(L"Box Mesh", box);
		auto rRes = dynamic_cast<Resource*>(res);
		rRes->mDirtyGPU = false;
		rRes->mDirtyDisk = false;
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::BoxMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(GenerateUuidFor("Cylinder Mesh"), L"Cylinder Mesh", GetNewId(), true);
		res->Create(L"Cylinder Mesh", cylinder);
		rRes = dynamic_cast<Resource*>(res);
		rRes->mDirtyGPU = false;
		rRes->mDirtyDisk = false;
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::CylinderMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(GenerateUuidFor("Geosphere Mesh"), L"Geosphere Mesh", GetNewId(), true);
		res->Create(L"Geosphere Mesh", geosphere);
		rRes = dynamic_cast<Resource*>(res);
		rRes->mDirtyGPU = false;
		rRes->mDirtyDisk = false;
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::GeosphereMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(GenerateUuidFor("Grid Mesh"), L"Grid Mesh", GetNewId(), true);
		res->Create(L"Grid Mesh", grid);
		rRes = dynamic_cast<Resource*>(res);
		rRes->mDirtyGPU = false;
		rRes->mDirtyDisk = false;
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::GridMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(GenerateUuidFor("Quad Mesh"), L"Quad Mesh", GetNewId(), true);
		res->Create(L"Quad Mesh", quad);
		rRes = dynamic_cast<Resource*>(res);
		rRes->mDirtyGPU = false;
		rRes->mDirtyDisk = false;
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::QuadMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(GenerateUuidFor("Sphere Mesh"), L"Sphere Mesh", GetNewId(), true);
		res->Create(L"Sphere Mesh", sphere);
		rRes = dynamic_cast<Resource*>(res);
		rRes->mDirtyGPU = false;
		rRes->mDirtyDisk = false;
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::SphereMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(GenerateUuidFor("Line Mesh"), L"Line Mesh", GetNewId(), true);
		res->Create(L"Line Mesh", line);
		rRes = dynamic_cast<Resource*>(res);
		rRes->mDirtyGPU = false;
		rRes->mDirtyDisk = false;
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::LineMesh, { ResourceType::Mesh, res->GetId() } });

		{
			auto defaultMeshHandle = CreateMaterial(GenerateUuidFor("Default Material"), L"Default Material", true, false);
			auto materialRes = (MaterialResource*)GetRawResource(defaultMeshHandle);
			auto mat = materialRes->ModifyData();
			mat->DifuseAlbedo = XMFLOAT4(Vector4(kOne));
			mat->FresnelR0 = XMFLOAT3(Vector3(kZero));
			mat->Roughness = 0.2f;
			rRes = dynamic_cast<Resource*>(res);
			rRes->mDirtyGPU = false;
			rRes->mDirtyDisk = false;
			materialRes->mDirtyDisk = materialRes->mDirtyGPU = false;
			mDefaultResourceMap.insert({ DefaultResource::DefaultMaterial, { ResourceType::Material, materialRes->GetId() } });
		}
	}

	ResourceHandle DResourceManager::CreateMesh(Uuid uuid, std::wstring const& path, bool isDefault, bool fromFile)
	{
		// TODO: Better allocation
		auto meshRes = new MeshResource(uuid, path, GetNewId(), isDefault);

		dynamic_cast<Resource*>(meshRes)->mLoaded = !fromFile;

		// Add the handle to path and resource maps
		ResourceHandle handle = { ResourceType::Mesh, meshRes->GetId() };
		UpdateMaps(meshRes);

		return handle;
	}

	ResourceHandle DResourceManager::CreateMaterial(std::wstring const& dirpath)
	{

		if (!D_H_ENSURE_DIR(dirpath))
			throw D_EXCEPTION::FileNotFoundException("Specified directory not found: " + STR_WSTR(dirpath));
		auto parent = Path(dirpath);

		auto path = parent.append(D_FILE::GetNewFileName(L"New Material", L".mat", parent));

		// Create resource
		auto handle = CreateMaterial(GenerateUuid(), path, false, false);
		auto res = GetRawResource(handle);
		D_RESOURCE_LOADER::SaveResource(res);
		return handle;
	}

	ResourceHandle DResourceManager::CreateMaterial(Uuid uuid, std::wstring const& path, bool isDefault, bool fromFile)
	{
		// TODO: Better allocation
		auto matRes = new MaterialResource(uuid, path, GetNewId(), isDefault);
		dynamic_cast<Resource*>(matRes)->mLoaded = !fromFile;

		// Add the handle to path and resource maps
		ResourceHandle handle = { ResourceType::Material, matRes->GetId() };
		UpdateMaps(matRes);

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
				if (resource->GetDirtyGPU())
					resource->UpdateGPU(context);
			}
		}
	}

	void DResourceManager::UpdateMaps(Resource* resource)
	{

		// Update resource map
		mResourceMap.at(resource->GetType()).try_emplace(resource->GetId(), resource);

		// Update uuid map
		//mUuidMap.try_emplace(resource->GetUuid(), resource);

		// Update path map
		mPathMap.try_emplace(resource->GetPath().wstring(), resource);
	}

	void DResourceManager::SaveAllResources()
	{
		for (auto& resType : mResourceMap)
		{
			for (auto& res : resType.second)
			{
				auto resource = res.second;
				if (resource->GetDirtyDisk())
					D_RESOURCE_LOADER::SaveResource(resource);
			}
		}
	}

	void DResourceManager::GetAllResources(DVector<Resource*>& resources)
	{
		for (auto& resType : mResourceMap)
		{
			for (auto& res : resType.second)
			{
				resources.push_back(res.second);
			}
		}
	}
}