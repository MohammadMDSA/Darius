#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "Scene.hpp"
#include "GameObject.hpp"

#include "EntityComponentSystem/Components/ComponentBase.hpp"
#include "EntityComponentSystem/Components/TransformComponent.hpp"

#include <Core/Filesystem/FileUtils.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Containers/Set.hpp>
#include <Core/Serialization/Json.hpp>
#include <Core/Uuid.hpp>
#include <Renderer/CommandContext.hpp>
#include <Utils/Assert.hpp>

#include <flecs.h>

#include <fstream>

using namespace D_MEMORY;
using namespace D_CORE;

namespace Darius::Scene
{
	std::unique_ptr<D_CONTAINERS::DSet<GameObject*>>					GOs = nullptr;
	std::unique_ptr<D_CONTAINERS::DMap<Uuid, GameObject*, UuidHasher>>	UuidMap = nullptr;
	std::unique_ptr<D_CONTAINERS::DMap<D_ECS::EntityId, GameObject*>>	EntityMap = nullptr;

	DVector<GameObject*>												ToBeDeleted;

	std::string															SceneName;
	D_FILE::Path														ScenePath;

	bool																Loaded = false;

	// Static Init
	D_ECS::Entity SceneManager::Root = D_ECS::Entity();
	D_ECS::ECSRegistry SceneManager::World = D_ECS::ECSRegistry();

	void SceneManager::Initialize()
	{
		D_ASSERT(!GOs);
		UuidMap = std::make_unique<D_CONTAINERS::DMap<Uuid, GameObject*, UuidHasher>>();
		GOs = std::make_unique<D_CONTAINERS::DSet<GameObject*>>();
		EntityMap = std::make_unique<D_CONTAINERS::DMap<D_ECS::EntityId, GameObject*>>();


#ifdef _DEBUG
		World.set<flecs::Rest>({});
#endif // _DEBUG

		Root = World.entity("Root");
	}

	void SceneManager::Shutdown()
	{
		D_ASSERT(GOs);

		GOs.reset();
		UuidMap.reset();
	}

	void SceneManager::Update(float deltaTime)
	{
		if (!GOs)
			return;

		World.progress(deltaTime);

		D_GRAPHICS::GraphicsContext& context = D_GRAPHICS::GraphicsContext::Begin(L"Updateing objects");

		context.PIXBeginEvent(L"Updating components");
		
		RemoveDeleted();

		for (auto const go : *GOs)
		{
			go->VisitComponents([deltaTime](D_ECS_COMP::ComponentBase* comp)
				{
					try
					{
						comp->Update(deltaTime);
					}
					catch (const std::exception& e)
					{
						D_LOG_ERROR("Exception during updating component " << comp->GetComponentName() << " : " << e.what());
					}
				});
		}

		context.PIXEndEvent();

		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, L"Update Mesh Constant Buffers");

		D_CONTAINERS::DSet<GameObject*>::iterator goIterator = GOs->begin();

		while (goIterator != GOs->end())
		{
			if ((*goIterator)->GetActive())
				(*goIterator)->Update(context, deltaTime);
			goIterator++;
		}

		RemoveDeleted();

		PIXEndEvent(context.GetCommandList());
		context.Finish();
	}

	bool SceneManager::Create(D_FILE::Path const& path)
	{
		if (Loaded)
			Unload();
		auto filename = D_FILE::GetFileName(path);
		SceneName = STR_WSTR(filename);

		ScenePath = path;

		Loaded = true;
		return true;
	}

	GameObject* SceneManager::CreateGameObject()
	{
		auto uuid = GenerateUuid();
		return CreateGameObject(uuid);
	}

	GameObject* SceneManager::CreateGameObject(Uuid uuid)
	{
		auto entity = World.entity().child_of(Root);
		D_LOG_DEBUG("Created entity: " << entity.id());

		// TODO: Better allocation
		auto go = new GameObject(uuid, entity);
		GOs->insert(go);

		// Update uuid map
		UuidMap->emplace(uuid, go);

		EntityMap->emplace(entity, go);

		go->Start();

		return go;
	}

	void SceneManager::DeleteGameObject(GameObject* go)
	{
		go->VisitChildren([&](GameObject* child)
			{
				DeleteGameObject(child);
			});

		go->mDeleted = true;
		ToBeDeleted.push_back(go);
	}

	void SceneManager::DeleteGameObjectData(GameObject* go)
	{
		go->OnDestroy();
		UuidMap->erase(go->GetUuid());
		EntityMap->erase(go->mEntity);
		go->mEntity.destruct();
		delete go;
	}

	// Slow function
	void SceneManager::GetGameObjects(D_CONTAINERS::DVector<GameObject*>& container)
	{
		for (auto& go : *GOs)
		{
			container.push_back(go);
		}
	}

	bool SceneManager::Load(std::wstring const& path)
	{
		D_SERIALIZATION::Json sceneJson;
		auto ifs = std::ifstream(path);

		if (!ifs)
			return false;

		ifs >> sceneJson;
		ifs.close();

		auto filePath = Path(path);
		auto filename = filePath.string();
		filename = filename.substr(0, filename.size() - filePath.extension().string().size());

		Create(filePath);

		// Loading Objects
		for (int i = 0; i < sceneJson["Objects"].size(); i++)
		{
			D_SERIALIZATION::Json jObj = sceneJson["Objects"][i];

			Uuid uuid;
			D_CORE::from_json(jObj["Uuid"], uuid);
			auto obj = CreateGameObject(uuid);

			from_json(jObj, *obj);
		}

		// Loading hierarchy
		for (auto [obUuid, childList] : sceneJson["Hierarchy"].items())
		{
			auto go = (*UuidMap)[FromString(obUuid)];
			for (int i = 0; i < childList.size(); i++)
			{
				Uuid childUuid;
				D_CORE::from_json(childList[i], childUuid);
				auto child = (*UuidMap)[childUuid];
				child->SetParent(go);
			}
		}

		// Loading Components
		for (auto& [objUuidStr, objCompsJ] : sceneJson["ObjectComponent"].items())
		{
			Uuid objUuid = FromString(objUuidStr);
			auto gameObject = (*UuidMap)[objUuid];

			for (auto& [compName, compJ] : objCompsJ.items())
			{
				auto compR = World.component(compName.c_str());
				auto compId = World.id(compR);
				
				// Adding component to entity
				gameObject->mEntity.add(compR);

				auto compP = gameObject->mEntity.get_mut(compId);
				
				// Get component pointer
				auto comp = reinterpret_cast<D_ECS_COMP::ComponentBase*>(compP);

				D_CORE::from_json(compJ["Uuid"], comp->mUuid);
				comp->mGameObject = gameObject;
				
				comp->Deserialize(compJ);
			}
		}

		StartScene();
	}

	bool SceneManager::Save()
	{
		D_SERIALIZATION::Json sceneJson = D_SERIALIZATION::Json::object();

		// Serializing objects and hierarchy
		DVector<GameObject> rawGos;
		for (GameObject const* go : *GOs)
		{
			auto bb = go->mEntity.has<D_ECS_COMP::TransformComponent>();
			rawGos.push_back(*go);


			// Serialize hierarchy
			Json& goContext = sceneJson["Hierarchy"][ToString(go->GetUuid())];
			go->VisitChildren([&](GameObject const* child)
				{
					goContext.push_back(ToString(child->GetUuid()));
					
				});
		}
		sceneJson["Objects"] = rawGos;

		// Serializing components
		for (GameObject const* go : *GOs)
		{
			D_SERIALIZATION::Json objectComps;
			go->VisitComponents([&](D_ECS_COMP::ComponentBase const* comp)
				{
					D_SERIALIZATION::Json componentJson;
					comp->Serialize(componentJson);
					D_CORE::to_json(componentJson["Uuid"], comp->mUuid);
					objectComps[comp->GetComponentName()] = componentJson;
				});
			sceneJson["ObjectComponent"][ToString(go->GetUuid())] = objectComps;
		}

		auto pathName = Path(ScenePath);

		auto ofs = std::ofstream(pathName);
		if (!ofs)
			return false;

		ofs << sceneJson;

		return true;
	}

	void SceneManager::Unload()
	{
		Root.children([&](D_ECS::Entity ent)
			{
				DeleteGameObject((*EntityMap)[ent]);
			});

		GOs->clear();

		SceneName = "";
		ScenePath = Path();
		Loaded = false;
	}

	GameObject* SceneManager::GetGameObject(D_ECS::Entity entity)
	{
		if (!EntityMap->contains(entity))
			return nullptr;
		return EntityMap->at(entity);
	}

	void SceneManager::StartScene()
	{
		for (auto go : *GOs)
		{
			go->Start();
		}
	}

	void SceneManager::RemoveDeleted()
	{

		// Delete game objects
		for (auto go : ToBeDeleted)
		{
			DeleteGameObjectData(go);
			GOs->erase(go);
		}

		ToBeDeleted.clear();
	}


	D_FILE::Path SceneManager::GetPath()
	{
		return ScenePath;
	}

	void SceneManager::SetPath(D_FILE::Path path)
	{
		ScenePath = path;
	}
	
	bool SceneManager::IsLoaded()
	{
		return Loaded;
	}

}