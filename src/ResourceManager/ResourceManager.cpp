#include "pch.hpp"
#include "ResourceManager.hpp"
#include "Resource.hpp"
#include "MeshResource.hpp"
#include "MaterialResource.hpp"

#include <Core/Containers/Map.hpp>
#include <Renderer/Geometry/GeometryGenerator.hpp>
#include <Utils/Assert.hpp>

#include <filesystem>

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

	ResourceHandle LoadResource(std::wstring path)
	{
		return _ResourceManager->LoadResource(path);
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

	Resource* _GetRawResource(ResourceHandle handle)
	{
		return _ResourceManager->GetRawResource(handle);
	}

	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type)
	{
		return _ResourceManager->GetResourcePreviews(type);
	}

	ResourceHandle GetDefaultResource(DefaultResource type)
	{
		return _ResourceManager->GetDefaultResource(type);
	}

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

		// TODO: bad allocation
		auto res = new MeshResource(L"", GetNewId(), true);
		res->Create(L"Box Mesh", box);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::BoxMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(L"", GetNewId(), true);
		res->Create(L"Cylinder Mesh", cylinder);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::CylinderMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(L"", GetNewId(), true);
		res->Create(L"Geosphere Mesh", geosphere);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::GeosphereMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(L"", GetNewId(), true);
		res->Create(L"Grid Mesh", grid);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::GridMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(L"", GetNewId(), true);
		res->Create(L"Quad Mesh", quad);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::QuadMesh, { ResourceType::Mesh, res->GetId() } });

		res = new MeshResource(L"", GetNewId(), true);
		res->Create(L"Sphere Mesh", sphere);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
		mDefaultResourceMap.insert({ DefaultResource::SphereMesh, { ResourceType::Mesh, res->GetId() } });

		{
			auto defaultMeshHandle = CreateMaterial(L"", true);
			auto materialRes = (MaterialResource*)GetRawResource(defaultMeshHandle);
			auto mat = materialRes->Get();
			mat->DifuseAlbedo = XMFLOAT4(Vector4(kOne));
			mat->FresnelR0 = XMFLOAT3(Vector3(kZero));
			mat->Roughness = 0.2f;
			mDefaultResourceMap.insert({ DefaultResource::DefaultMaterial, { ResourceType::Material, materialRes->GetId() } });
		}
	}

	ResourceHandle DResourceManager::LoadResource(std::wstring path)
	{
		// Check if we already have the resource
		if (mPathMap.contains(path))
		{
			auto handle = mPathMap.at(path);
			auto resource = GetRawResource(handle);
			if (!resource->GetLoaded())
				resource->Load();
			return handle;
		}

		if (std::filesystem::exists(path) && !std::filesystem::is_directory(path))
		{
			auto extension = std::filesystem::path(path).extension();
			// Checking for supported resource
			if (extension == ".fbx")
				return CreateMesh(path);
			if (extension == ".mat")
				return CreateMaterial(path);
			else
				return { ResourceType::None, 0 };

		}

		return { ResourceType::None, 0 };
	}

	ResourceHandle DResourceManager::CreateMesh(std::wstring path)
	{
		return CreateMesh(path, false);
	}

	ResourceHandle DResourceManager::CreateMesh(std::wstring path, bool isDefault)
	{
		// TODO: Better allocation
		auto meshRes = new MeshResource(path, GetNewId(), isDefault);

		// Trying to load mesh
		if (!meshRes->Load())
			return { ResourceType::None, 0 };

		// Add the handle to path and resource maps
		ResourceHandle handle = { ResourceType::Mesh, meshRes->GetId() };
		mPathMap.insert({ path, handle });
		AddMeshResource(meshRes);

		return handle;
	}

	ResourceHandle DResourceManager::CreateMaterial(std::wstring path)
	{
		return CreateMaterial(path, false);
	}

	ResourceHandle DResourceManager::CreateMaterial(std::wstring path, bool isDefault)
	{
		// TODO: Better allocation
		auto matRes = new MaterialResource(path, GetNewId(), isDefault);

		// Try to load material
		if (!matRes->Load())
			return { ResourceType::None, 0 };

		// Add the handle to path and resource maps
		ResourceHandle handle = { ResourceType::Material, matRes->GetId() };
		mPathMap.insert({ path, handle });
		AddMaterialResource(matRes);

		return handle;
	}

	ResourceHandle DResourceManager::GetDefaultResource(DefaultResource type)
	{
		return mDefaultResourceMap.at(type);
	}
}