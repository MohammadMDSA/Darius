#pragma once

#include "GameObject.hpp"

#include <core/Containers/Set.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{
	void Initialize();
	void Shutdown();

	GameObject* CreateGameObject();
	D_CONTAINERS::DSet<GameObject*> const* GetGameObjects();

	bool Create(std::string const& name);
	void Unload();
	bool Load(std::wstring const& path);
	bool Save(std::string const& name, std::wstring& path);

}