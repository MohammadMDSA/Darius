#include "pch.hpp"
#include "Scene.hpp"

#include "GameObject.hpp"

#include "EntityComponentSystem/Components/ComponentBase.hpp"
#include "EntityComponentSystem/Components/BehaviourComponent.hpp"
#include "EntityComponentSystem/Components/TransformComponent.hpp"
#include "Resources/PrefabResource.hpp"

#include <Core/Containers/Set.hpp>
#include <Core/Filesystem/FileUtils.hpp>
#include <Core/Memory/Memory.hpp>
#include <Core/Memory/Allocators/PagedAllocator.hpp>
#include <Core/Serialization/Json.hpp>
#include <Core/Serialization/TypeSerializer.hpp>
#include <Core/Uuid.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <Job/Job.hpp>
#include <Utils/Assert.hpp>

#include <flecs.h>

#include <fstream>

using namespace D_CONTAINERS;
using namespace D_CORE;
using namespace D_FILE;
using namespace D_MEMORY;
using namespace D_SERIALIZATION;

namespace Darius::Scene
{
	std::unique_ptr<D_CONTAINERS::DSet<GameObject*>>					GOs = nullptr;
	std::unique_ptr<D_CONTAINERS::DUnorderedMap<Uuid, GameObject*, UuidHasher>>	UuidMap = nullptr;
	std::unique_ptr<D_CONTAINERS::DUnorderedMap<D_ECS::EntityId, GameObject*>>	EntityMap = nullptr;

	DVector<std::function<void(float, D_ECS::ECSRegistry&)>>			BehaviourUpdaterFunctions;
	DVector<std::function<void(float, D_ECS::ECSRegistry&)>>			BehaviourLateUpdaterFunctions;
	DUnorderedMap<D_ECS::ComponentEntry, rttr::type, std::hash<flecs::id_t>> ComponentEntityReflectionTypeMapping;


	DVector<DVector<GameObject*>>										ToBeDeleted;
	DVector<GameObject*>												ToBeStarted;
	DSet<GameObject*>													DeletedObjects;

	std::string															SceneName;
	D_FILE::Path														ScenePath;

	bool																Loaded = false;
	bool																Started = false;
	bool																Running = false;

	// Static Init
	D_ECS::Entity SceneManager::Root = D_ECS::Entity();
	D_ECS::ECSRegistry SceneManager::World = D_ECS::ECSRegistry();
	DUnorderedMap<D_CORE::StringId, D_ECS::ComponentEntry> SceneManager::ComponentEntryCache;

	D_CORE::Signal<void()> SceneManager::OnSceneCleared;
	D_MEMORY::PagedAllocator<GameObject, true>							GoAllocator;

	void SceneManager::Initialize()
	{
		D_ASSERT(!GOs);
		UuidMap = std::make_unique<D_CONTAINERS::DUnorderedMap<Uuid, GameObject*, UuidHasher>>();
		GOs = std::make_unique<D_CONTAINERS::DSet<GameObject*>>();
		EntityMap = std::make_unique<D_CONTAINERS::DUnorderedMap<D_ECS::EntityId, GameObject*>>();

		// Registering components
		D_MATH::TransformComponent::StaticConstructor();
		D_ECS_COMP::BehaviourComponent::StaticConstructor();

#ifdef _DEBUG
		//World.set<flecs::Rest>({});
#endif // _DEBUG

		PrefabResource::Register();

		ToBeDeleted.resize(D_GRAPHICS_DEVICE::gNumFrameResources);

		Root = World.entity("Root");

		Started = true;
		Running = false;

	}

	void SceneManager::Shutdown()
	{
		D_ASSERT(GOs);

		GoAllocator.Reset();
		GOs.reset();
		UuidMap.reset();
	}

	void SceneManager::LoadPrefabs()
	{
		auto prevs = D_RESOURCE::GetResourcePreviews(PrefabResource::GetResourceType());

		for(auto const& prev : prevs)
		{
			D_RESOURCE_LOADER::LoadResourceSync(prev.Path, false, true);
		}
	}

	void SceneManager::FrameInitialization()
	{
		World.each([&](D_MATH::TransformComponent& trans)
			{
				trans.SetClean();
			});
	}

	void SceneManager::Update(float deltaTime)
	{
		if(!GOs)
			return;

		World.progress(deltaTime);

		RemoveDeleted();

		// Start to-be-started objects
		for(auto go : ToBeStarted)
			go->Start();
		ToBeStarted.clear();

		// Update each component
		for(auto& updater : BehaviourUpdaterFunctions)
			updater(deltaTime, World);

	}

	void SceneManager::LateUpdate(float deltaTime)
	{
		RemoveDeleted();

		for(auto& updater : BehaviourLateUpdaterFunctions)
			updater(deltaTime, World);
	}

	bool SceneManager::Create(D_FILE::Path const& path)
	{
		if(Loaded)
			Unload();
		auto filename = D_FILE::GetFileName(path);
		SceneName = WSTR2STR(filename);

		ScenePath = path;

		Loaded = true;
		return true;
	}

	GameObject* SceneManager::AddGameObject(D_CORE::Uuid const& uuid, bool addToScene)
	{
		// Uuid must not already exist
		D_ASSERT(!UuidMap->contains(uuid));

		D_ECS::Entity entity;

		if(addToScene)
			entity = World.entity().child_of(Root);
		else
			entity = World.entity();

		D_LOG_DEBUG("Created entity: " << entity.id());

		// TODO: Better allocation
		auto go = GoAllocator.Alloc(std::move(uuid), std::move(entity), std::move(addToScene));

		if(addToScene)
			GOs->insert(go);

		// Update maps
		UuidMap->emplace(uuid, go);
		EntityMap->emplace(entity, go);

		return go;
	}

	GameObject* SceneManager::InstantiateGameObject(GameObject* go, bool maintainContext)
	{
		Json refGoJson;
		GameObject* result;
		DumpGameObject(go, refGoJson, maintainContext);
		LoadGameObject(refGoJson, &result, true);

		if(!go->IsInScene())
			result->mPrefab = go->GetUuid();

		return result;
	}

	GameObject* SceneManager::CreateGameObject(bool addToScene)
	{
		auto uuid = GenerateUuid();
		return CreateGameObject(uuid, addToScene);
	}

	GameObject* SceneManager::CreateGameObject(Uuid uuid, bool addToScene)
	{
		auto go = AddGameObject(uuid, addToScene);

		if(Started)
			go->Awake();
		if(Running)
			go->Start();

		return go;
	}

	void SceneManager::DeleteGameObject(GameObject* go)
	{
		if(go->mDeleted || !go->mEntity.is_valid())
			return;

		if(!go->mDeleted)
			go->OnPreDestroy();

		go->VisitChildren([&](GameObject* child)
			{
				DeleteGameObject(child);
			});

		go->mDeleted = true;
		UINT index = (D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex() + D_GRAPHICS_DEVICE::gNumFrameResources - 1) % D_GRAPHICS_DEVICE::gNumFrameResources;
		ToBeDeleted[index].push_back(go);
	}

	void SceneManager::DeleteGameObjectImmediately(GameObject* go)
	{
		DeleteGameObject(go);

		RemoveDeleted(true);
		World.progress();
	}

	void SceneManager::DeleteGameObjectData(GameObject* go)
	{
		go->OnDestroy();
		UuidMap->erase(go->GetUuid());
		EntityMap->erase(go->mEntity);
		go->mEntity.destruct();
		DeletedObjects.insert(go);
	}

	void SceneManager::RemoveDeletedPointers()
	{
		for(GameObject* go : DeletedObjects)
			GoAllocator.Free(go);

		DeletedObjects.clear();
	}

	// Slow function
	void SceneManager::GetGameObjects(D_CONTAINERS::DVector<GameObject*>& container)
	{
		UINT index = 0;
		container.resize(GOs->size());
		for(auto& go : *GOs)
		{
			container[index++] = go;
		}
	}

	bool SceneManager::Load(std::wstring const& path)
	{
		D_SERIALIZATION::Json sceneJson;
		auto ifs = std::ifstream(path);

		if(!ifs)
			return false;

		ifs >> sceneJson;
		ifs.close();

		auto filePath = Path(path);
		auto filename = filePath.string();
		filename = filename.substr(0, filename.size() - filePath.extension().string().size());

		Create(filePath);

		LoadSceneDump(sceneJson);

		return true;
	}

	bool SceneManager::Save()
	{
		D_SERIALIZATION::Json sceneJson = D_SERIALIZATION::Json::object();

		DumpScene(sceneJson);

		auto pathName = Path(ScenePath);

		auto ofs = std::ofstream(pathName);
		if(!ofs)
			return false;

		ofs << sceneJson;

		return true;
	}

	void SceneManager::DumpScene(Json& sceneJson)
	{
		// Serializing objects and hierarchy
		DVector<GameObject const*> rawGos;
		for(GameObject const* go : *GOs)
		{
			rawGos.push_back(go);


			// Serialize hierarchy
			Json& goContext = sceneJson["Hierarchy"][ToString(go->GetUuid())];
			go->VisitChildren([&](GameObject const* child)
				{
					goContext.push_back(ToString(child->GetUuid()));

				});
		}

		D_SERIALIZATION::SerializeSequentialContainer(rawGos, sceneJson["Objects"]);

		// Serializing components
		for(GameObject const* go : *GOs)
		{
			D_SERIALIZATION::Json objectComps;
			go->VisitComponents([&](D_ECS_COMP::ComponentBase const* comp)
				{
					auto compName = comp->GetComponentName().string();
					D_SERIALIZATION::Json componentJson;
					D_SERIALIZATION::Serialize(comp, componentJson);
					comp->OnSerialized();
					D_CORE::to_json(componentJson["Uuid"], comp->mUuid);
					objectComps[compName] = componentJson;
				});
			sceneJson["ObjectComponent"][ToString(go->GetUuid())] = objectComps;
		}
	}

	void SceneManager::LoadSceneDump(Json const& sceneJson)
	{
		Started = false;
		Running = false;

		// Loading Objects
		if(sceneJson.contains("Objects"))
			for(int i = 0; i < sceneJson["Objects"].size(); i++)
			{
				D_SERIALIZATION::Json const& jObj = sceneJson["Objects"][i];

				Uuid uuid;
				D_CORE::from_json(jObj["Uuid"], uuid);
				auto obj = CreateGameObject(uuid);

				D_SERIALIZATION::Deserialize(obj, jObj);
			}

		// Loading hierarchy
		if(sceneJson.contains("Hierarchy"))
			for(auto& [obUuid, childList] : sceneJson["Hierarchy"].items())
			{
				auto go = (*UuidMap)[FromString(obUuid)];
				for(int i = 0; i < childList.size(); i++)
				{
					Uuid childUuid;
					D_CORE::from_json(childList[i], childUuid);
					auto child = (*UuidMap)[childUuid];
					child->SetParent(go, GameObject::AttachmentType::KeepLocal);
				}
			}

		// Loading Components
		if(sceneJson.contains("ObjectComponent"))
			for(auto const& [objUuidStr, objCompsJ] : sceneJson["ObjectComponent"].items())
			{
				Uuid objUuid = FromString(objUuidStr);
				auto gameObject = (*UuidMap)[objUuid];

				for(auto const& [compName, compJ] : objCompsJ.items())
				{
					auto compR = World.component(compName.c_str());
					auto compId = World.id(compR);

					// Adding component to entity
					gameObject->mEntity.add(compR);

					auto compP = const_cast<void*>(gameObject->mEntity.get(compId));

					// Get component pointer
					auto comp = reinterpret_cast<D_ECS_COMP::ComponentBase*>(compP);

					D_CORE::from_json(compJ["Uuid"], comp->mUuid);

					gameObject->AddComponentRoutine(comp);

					comp->OnPreDeserialize();
					D_SERIALIZATION::Deserialize(comp, compJ);
					comp->OnDeserialized();
				}
			}

		World.progress();

		StartScene();
	}

	void SceneManager::SetDeferEnable(bool value)
	{
		if(value)
			World.defer_resume();
		else
			World.defer_suspend();
	}

	void SceneManager::LoadGameObject(Json const& json, GameObject** go, bool addToScene)
	{

		D_ASSERT(json.contains("Root"));
		D_ASSERT(json.contains("Hierarchy"));
		D_ASSERT(json.contains("ObjectComponent"));
		D_ASSERT(json.contains("Objects"));
		D_ASSERT(json.contains("Type"));
		D_ASSERT(json.at("Type") == "GameObject");

		Uuid rootUuid;
		D_CORE::UuidFromJson(rootUuid, json["Root"]);

		DUnorderedMap<Uuid, GameObject*, UuidHasher> addedObjs;

		// Creating game object instances
		for(int i = 0; i < json["Objects"].size(); i++)
		{
			Json const& objJson = json["Objects"][i];

			Uuid objUuid;
			D_CORE::UuidFromJson(objUuid, objJson["Uuid"]);
			auto obj = AddGameObject(objUuid, addToScene);

			D_SERIALIZATION::Deserialize(obj, objJson);
			addedObjs[objUuid] = obj;
		}

		// Loading hierarchy
		for(auto& [obUuidStr, childrenList] : json["Hierarchy"].items())
		{
			auto parentGo = addedObjs[FromString(obUuidStr)];
			for(int i = 0; i < childrenList.size(); i++)
			{
				Uuid childUuid;
				D_CORE::UuidFromJson(childUuid, childrenList[i]);
				auto childObj = addedObjs[childUuid];
				childObj->SetParent(parentGo, GameObject::AttachmentType::KeepLocal);
			}
		}

		// Loading Components
		for(auto const& [objUuidStr, objCompsJson] : json["ObjectComponent"].items())
		{
			Uuid objUuid = D_CORE::FromString(objUuidStr);
			auto gameObject = addedObjs[objUuid];

			for(auto const& [compName, compJson] : objCompsJson.items())
			{
				auto compR = World.component(compName.c_str());
				auto compId = World.id(compR);

				// Adding component to entity
				gameObject->mEntity.add(compR);
				auto compP = const_cast<void*>(gameObject->mEntity.get(compId));
				D_ASSERT(compP);

				// Get component pointer
				auto comp = reinterpret_cast<D_ECS_COMP::ComponentBase*>(compP);

				D_CORE::UuidFromJson(comp->mUuid, compJson["Uuid"]);
				gameObject->AddComponentRoutine(comp);

				D_SERIALIZATION::Deserialize(comp, compJson);
				comp->OnDeserialized();
			}
		}

		for(auto& [_, go] : addedObjs)
		{
			go->Awake();
			ToBeStarted.push_back(go);
		}

		auto rootGameObject = addedObjs[rootUuid];

		if(!addToScene)
			rootGameObject->mPrefab = rootUuid;

		*go = rootGameObject;
	}

	void SceneManager::DumpGameObject(GameObject const* go, _OUT_ D_SERIALIZATION::Json& json, bool maintainContext)
	{
		D_ASSERT(go);

		DVector<GameObject const*> toBeSerialized;

		DUnorderedMap<Uuid, Uuid, UuidHasher> newReferenceMap;
#define NEW_UUID(gameObject) newReferenceMap.at(gameObject->GetUuid())

		Serialization::SerializationContext serializationContext = {
			.Rereference = true,
			.MaintainExternalReferences = maintainContext,
			.ReferenceMap = newReferenceMap};

		toBeSerialized.push_back(go);
		go->VisitDescendants([&toBeSerialized](auto go)
			{
				toBeSerialized.push_back(go);
			});

		// Create uuid maps for rereferencing
		for(GameObject const* go : toBeSerialized)
		{
			auto goUuid = D_CORE::GenerateUuid();
			newReferenceMap[go->GetUuid()] = goUuid;
		}

		D_CORE::UuidToJson(NEW_UUID(go), json["Root"]);

		for(GameObject const* go : toBeSerialized)
		{
			// Serialize hierarchy
			Json& goContext = json["Hierarchy"][ToString(NEW_UUID(go))];
			go->VisitChildren([&](GameObject const* child)
				{
					goContext.push_back(ToString(NEW_UUID(child)));

				});

			// Serializing components
			D_SERIALIZATION::Json objectComps;
			go->VisitComponents([&](D_ECS_COMP::ComponentBase const* comp)
				{
					auto newCompUuid = D_CORE::GenerateUuid();
					newReferenceMap[comp->mUuid] = newCompUuid;

					D_SERIALIZATION::Json componentJson;
					D_SERIALIZATION::Serialize(comp, componentJson, serializationContext);
					comp->OnSerialized();
					D_CORE::UuidToJson(newCompUuid, componentJson["Uuid"]);
					objectComps[comp->GetComponentName().string()] = componentJson;
				});
			json["ObjectComponent"][ToString(NEW_UUID(go))] = objectComps;
		}

		D_SERIALIZATION::SerializeSequentialContainer(toBeSerialized, json["Objects"], serializationContext);

		// Replacing objects uuid with the new ones
		for(int i = 0; i < json["Objects"].size(); i++)
		{
			Uuid currentUuid;
			D_CORE::UuidFromJson(currentUuid, json["Objects"][i]["Uuid"]);
			D_CORE::UuidToJson(newReferenceMap[currentUuid], json["Objects"][i]["Uuid"]);
		}

		json["Type"] = "GameObject";

#undef NEW_UUID
	}

	void SceneManager::Unload()
	{
		ClearScene();
		SceneName = "";
		ScenePath = Path();
		Loaded = false;
	}

	void SceneManager::ClearScene(std::function<void()> preClean)
	{
		Root.children([&](D_ECS::Entity ent)
			{
				DeleteGameObject((*EntityMap)[ent]);
			});

		if(preClean)
			preClean();

		RemoveDeleted(true);
		GOs->clear();
		World.progress();
		Running = false;

		OnSceneCleared();
	}

	GameObject* SceneManager::GetGameObject(D_ECS::Entity entity)
	{
		if(!EntityMap->contains(entity))
			return nullptr;
		return EntityMap->at(entity);
	}

	GameObject* SceneManager::GetGameObject(D_CORE::Uuid const& uuid)
	{
		if(!UuidMap->contains(uuid))
			return nullptr;

		return UuidMap->at(uuid);
	}

	void SceneManager::RemoveDeleted(bool flush)
	{
		if(flush)
		{
			D_GRAPHICS::GetCommandManager()->IdleGPU();
			for(auto& instance : ToBeDeleted)
			{
				for(auto go : instance)
				{
					DeleteGameObjectData(go);
					GOs->erase(go);
				}
				instance.clear();
			}
		}
		else
		{
			auto currentResourceIndex = D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex();
			for(auto go : ToBeDeleted[currentResourceIndex])
			{
				DeleteGameObjectData(go);
				GOs->erase(go);
			}

			ToBeDeleted[currentResourceIndex].clear();
		}
		RemoveDeletedPointers();
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

	void SceneManager::StartScene()
	{
		if(Started)
			return;

		Started = true;

		for(auto& [_, go] : *UuidMap)
		{
			go->Awake();
		}
	}

	void SceneManager::SetAwake()
	{
		Running = true;

		for(auto go : *GOs)
		{
			go->Start();
		}
	}

	void SceneManager::RegisterComponentUpdater(std::function<void(float, D_ECS::ECSRegistry&)> updater)
	{
		BehaviourUpdaterFunctions.push_back(updater);
	}

	void SceneManager::RegisterComponentLateUpdater(std::function<void(float, D_ECS::ECSRegistry&)> updater)
	{
		BehaviourLateUpdaterFunctions.push_back(updater);
	}

	bool SceneManager::IsRunning()
	{
		return Running;
	}

	rttr::type SceneManager::GetComponentReflectionTypeByComponentEntity(ECS::ComponentEntry comp)
	{
		auto result = ComponentEntityReflectionTypeMapping.find(comp);

		if(result == ComponentEntityReflectionTypeMapping.end())
		{
			D_ASSERT_M(false, "Given component is not registered via its static constructor.");
			return rttr::type::get<rttr::detail::invalid_type>();
		}

		return result->second;
	}

	void SceneManager::RegisterComponentType(D_ECS::ComponentEntry componentId, rttr::type type)
	{
		ComponentEntityReflectionTypeMapping.emplace(componentId, type);
	}

}