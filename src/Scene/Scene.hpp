#pragma once

#include "EntityComponentSystem/Entity.hpp"

#include <Core/Uuid.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/Filesystem/Path.hpp>
#include <Utils/Assert.hpp>

#include <rttr/type.h>

#include <functional>

#ifndef D_WORLD
#define D_WORLD Darius::Scene::SceneManager
#endif // !D_WORLD

namespace Darius::Scene::ECS::Components
{
	class BehaviourComponent;
}

namespace Darius::Scene
{

	class GameObject;

	class SceneManager
	{
	public:
		static void				Initialize();
		static void				Shutdown();

		static GameObject*		CreateGameObject(bool addToScene = true);
		static GameObject*		CreateGameObject(D_CORE::Uuid uuid, bool addToScene = true);
		static GameObject*		InstantiateGameObject(GameObject* go, bool maintainContext = true);
		static void				DeleteGameObject(GameObject* go);
		// Use of this method is strongly discouraged
		static void				DeleteGameObjectImmediately(GameObject* go);
		static void				GetGameObjects(D_CONTAINERS::DVector<GameObject*>& container);
		static GameObject*		GetGameObject(D_CORE::Uuid const& uuid);
		static GameObject*		GetGameObject(D_ECS::Entity entity);

		static void				DumpGameObject(GameObject const* go, _OUT_ D_SERIALIZATION::Json& json, bool maintainContext = false);
		static void				LoadGameObject(D_SERIALIZATION::Json const& json, _OUT_ GameObject** go, bool addToScene = true);

		static void				FrameInitialization();
		static void				Update(float deltaTime);
		static void				LateUpdate(float deltaTime);

		static bool				Create(D_FILE::Path const& path);
		static void				Unload();
		static void				ClearScene(std::function<void()> preClean = nullptr);
		static bool				Load(std::wstring const& path);
		static bool				Save();
		static D_FILE::Path		GetPath();
		static void				SetPath(D_FILE::Path path);
		static bool				IsLoaded();
		static bool				IsRunning();

		static INLINE D_ECS::Entity	GetRoot() { return Root; }
		
		template<typename COMP>
		static INLINE void		IterateComponents(std::function<void(COMP&)> callback)
		{
			World.each(FLECS_MOV(callback));
		}

		template<typename COMP>
		static INLINE UINT		CountComponents()
		{
			return (UINT)World.count<COMP>();
		}

		template<typename T>
		static INLINE D_ECS::ECSId GetTypeId()
		{
			return World.id<T>();
		}

		static rttr::type GetComponentReflectionTypeByComponentEntity(ECS::ComponentEntry comp);

		static INLINE bool		IsIdValid(D_ECS::ECSId id)
		{
			return World.is_valid(id);
		}

		static INLINE D_ECS::ComponentEntry GetComponentEntity(std::string const& compName)
		{
			return World.component(compName.c_str());
		}

		template<class COMP, class PARENT>
		static INLINE D_ECS::EntityId RegisterComponent()
		{
			auto comp = World.component<COMP>(COMP::ClassName().c_str());
			auto parentComp = World.component<PARENT>();
			RegisterComponentType(comp, rttr::type::get<COMP>());
			D_ASSERT(World.is_valid(parentComp));
			comp.is_a(parentComp);
			return comp;
		}

		template<class COMP>
		static INLINE D_ECS::EntityId RegisterComponent()
		{
			return World.component<COMP>(COMP::ClassName().c_str());

		}

		// Don't call!
		static void				RegisterComponentUpdater(std::function<void(float, D_ECS::ECSRegistry&)> updater);
		static void				RegisterComponentLateUpdater(std::function<void(float, D_ECS::ECSRegistry&)> updater);

		static void				SetAwake();

	public:
		// Dumping and reloading scene for simulation
		static void				DumpScene(D_SERIALIZATION::Json& sceneDump);
		static void				LoadSceneDump(D_SERIALIZATION::Json const& sceneDump);

	private:
		static void				DeleteGameObjectData(GameObject* go);
		static void				RemoveDeletedPointers();
		static void				StartScene();
		static void				RemoveDeleted();
		static void				RegisterComponentType(D_ECS::ComponentEntry componentId, rttr::type type);

		static GameObject*		AddGameObject(D_CORE::Uuid const& uuid, bool addToScene = true);

		static D_ECS::Entity	Root;
		static D_ECS::ECSRegistry World;

	};

}