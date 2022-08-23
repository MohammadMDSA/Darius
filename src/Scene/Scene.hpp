#pragma once

#include "GameObject.hpp"

#include <core/Containers/Vector.hpp>

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
		static D_CONTAINERS::DVector<GameObject*> const* GetGameObjects();

		static void				Update(float deltaTime);

		static bool				Create(std::string const& name);
		static void				Unload();
		static bool				Load(std::wstring const& path);
		static bool				Save(std::string const& name, std::wstring& path);
	};

}