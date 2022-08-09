#pragma once

#include "GameObject.hpp"

#include <core/Containers/Vector.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{
	void Initialize();
	void Shutdown();

	GameObject* CreateGameObject();
	D_CONTAINERS::DVector<GameObject*> const* GetGameObjects();

	void Update(float deltaTime);

	bool Create(std::string const& name);
	void Unload();
	bool Load(std::wstring const& path);
	bool Save(std::string const& name, std::wstring& path);

	class SceneManager
	{
	public:
		void		Update(float deltaTime);
	};

}