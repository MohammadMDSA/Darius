#pragma once

#include "EntityComponentSystem/Entity.hpp"

#include <Core/Uuid.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/Filesystem/Path.hpp>

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

		static GameObject*		CreateGameObject();
		static GameObject*		InstantiateGameObject(GameObject* go, bool maintainContext = true);
		static void				DeleteGameObject(GameObject* go);
		static void				DeleteGameObjectImmediately(GameObject* go);
		static void				GetGameObjects(D_CONTAINERS::DVector<GameObject*>& container);
		static GameObject*		GetGameObject(D_CORE::Uuid const& uuid);
		static GameObject*		GetGameObject(D_ECS::Entity entity);

		static void				DumpGameObject(GameObject const* go, _OUT_ D_SERIALIZATION::Json& json, bool maintainContext = false);
		static void				LoadGameObject(D_SERIALIZATION::Json const& json, _OUT_ GameObject** go, bool addToScene = true);

		static void				Update(float deltaTime);
		static void				LateUpdate(float deltaTime);

		static bool				Create(D_FILE::Path const& path);
		static void				Unload();
		static void				ClearScene();
		static bool				Load(std::wstring const& path);
		static bool				Save();
		static D_FILE::Path		GetPath();
		static void				SetPath(D_FILE::Path path);
		static bool				IsLoaded();
		static bool				IsRunning();

		static INLINE D_ECS::Entity	GetRoot() { return Root; }
		static INLINE D_ECS::ECSRegistry& GetRegistry() { return World; }

		// Don't call!
		static void				RegisterComponentUpdater(std::function<void(float, D_ECS::ECSRegistry&)> updater);
		static void				RegisterComponentLateUpdater(std::function<void(float, D_ECS::ECSRegistry&)> updater);

		static void				SetAwake();

#ifdef _D_EDITOR
	public:
#elif
	private:
#endif
		// Dumping and reloading scene for simulation
		static void				DumpScene(D_SERIALIZATION::Json& sceneDump);
		static void				LoadSceneDump(D_SERIALIZATION::Json const& sceneDump);

	private:
		static void				DeleteGameObjectData(GameObject* go);
		static void				RemoveDeletedPointers();
		static GameObject*		CreateGameObject(D_CORE::Uuid uuid);
		static void				StartScene();
		static void				RemoveDeleted();

		static GameObject*		AddGameObject(D_CORE::Uuid const& uuid, bool addToScene = true);

		static D_ECS::Entity	Root;
		static D_ECS::ECSRegistry World;

	};

}