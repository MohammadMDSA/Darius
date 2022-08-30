#pragma once

#include "GameObject.hpp"
#include "EntityComponentSystem/Entity.hpp"

#include <Core/Containers/Vector.hpp>
#include <Core/Filesystem/Path.hpp>

#ifndef D_WORLD
#define D_WORLD Darius::Scene::SceneManager
#endif // !D_WORLD

namespace Darius::Scene
{

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

		static bool				Create(std::string const& name);
		static void				Unload();
		static bool				Load(std::wstring const& path);
		static bool				Save(std::string const& name, D_FILE::Path const& path);
		
		static D_ECS::ECSRegistry& GetRegistry();

	private:
		static void				DeleteGameObjectData(GameObject* go);
		static GameObject*		CreateGameObject(Uuid uuid);
		static void				StartScene();
	};

}