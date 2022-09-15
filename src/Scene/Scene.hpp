#pragma once

#include "EntityComponentSystem/Entity.hpp"

#include <Core/Uuid.hpp>
#include <Core/Containers/Vector.hpp>
#include <Core/Filesystem/Path.hpp>

#ifndef D_WORLD
#define D_WORLD Darius::Scene::SceneManager
#endif // !D_WORLD

using namespace D_CORE;

namespace Darius::Scene
{

	class GameObject;

	class SceneManager
	{
	public:
		static void				Initialize();
		static void				Shutdown();

		static GameObject*		CreateGameObject();
		static void				DeleteGameObject(GameObject* go);
		static void				GetGameObjects(D_CONTAINERS::DVector<GameObject*>& container);
		
		static GameObject*		GetGameObject(D_ECS::Entity entity);

		static void				Update(float deltaTime);

		static bool				Create(D_FILE::Path const& path);
		static void				Unload();
		static bool				Load(std::wstring const& path);
		static bool				Save();
		static D_FILE::Path		GetPath();
		static void				SetPath(D_FILE::Path path);
		static bool				IsLoaded();
		
		static INLINE D_ECS::Entity	GetRoot() { return Root; }
		static INLINE D_ECS::ECSRegistry& GetRegistry() { return World; }

	private:
		static void				DeleteGameObjectData(GameObject* go);
		static GameObject*		CreateGameObject(Uuid uuid);
		static void				StartScene();
		static void				RemoveDeleted();

		static D_ECS::Entity	Root;
		static D_ECS::ECSRegistry World;

	};

}