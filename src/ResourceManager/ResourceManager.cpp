#include "pch.hpp"
#include "ResourceManager.hpp"
#include "Resource.hpp"
#include "MeshResource.hpp"

#include <Core/Containers/Map.hpp>
#include <Renderer/Geometry/GeometryGenerator.hpp>
#include <Utils/Assert.hpp>

using namespace D_CONTAINERS;

namespace Darius::ResourceManager
{
	bool _initialized = false;

	std::unique_ptr<DResourceManager>				_ResourceManager;
	const DMap<std::string, ResourceType>			ResourceTypeMap =
	{
		{ MeshResource::GetTypeName(), ResourceType::Mesh }
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

	Resource* _GetRawResource(ResourceHandle handle)
	{
		return _ResourceManager->GetRawResource(handle);
	}

	D_CONTAINERS::DVector<ResourcePreview> GetResourcePreviews(ResourceType type)
	{
		return _ResourceManager->GetResourcePreviews(type);
	}

	DResourceManager::DResourceManager()
	{
		mResourceMap.insert({ ResourceType::Mesh, DMap<DResourceId, Resource*>() });

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
		auto id = GetNewId();

		auto box = D_RENDERER_GEOMETRY_GENERATOR::CreateBox(1.f, 1.f, 1.f, 1);
		auto cylinder = D_RENDERER_GEOMETRY_GENERATOR::CreateCylinder(0.5f, 0.5f, 1, 20, 20);
		auto geosphere = D_RENDERER_GEOMETRY_GENERATOR::CreateGeosphere(0.5f, 20);
		auto grid = D_RENDERER_GEOMETRY_GENERATOR::CreateGrid(100.f, 100.f, 100, 100);
		auto quad = D_RENDERER_GEOMETRY_GENERATOR::CreateQuad(1.f, 1.f, 1.f, 1.f, 1.f);
		auto sphere = D_RENDERER_GEOMETRY_GENERATOR::CreateSphere(0.5f, 20, 20);

		// TODO: bad allocation
		auto res = new MeshResource(GetNewId());
		res->Create(L"Box Mesh", box);
		mResourceMap.at(ResourceType::Mesh).insert({res->GetId(), res});

		res = new MeshResource(GetNewId());
		res->Create(L"Cylinder Mesh", cylinder);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });

		res = new MeshResource(GetNewId());
		res->Create(L"Geosphere Mesh", geosphere);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });

		res = new MeshResource(GetNewId());
		res->Create(L"Grid Mesh", grid);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });

		res = new MeshResource(GetNewId());
		res->Create(L"Quad Mesh", quad);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });

		res = new MeshResource(GetNewId());
		res->Create(L"Sphere Mesh", sphere);
		mResourceMap.at(ResourceType::Mesh).insert({ res->GetId(), res });
	}

}