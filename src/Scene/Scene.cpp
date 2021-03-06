#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "Scene.hpp"
#include "GameObject.hpp"

#include <Core/Memory/Memory.hpp>
#include <Utils/Assert.hpp>

using namespace D_MEMORY;

namespace Darius::Scene
{
	bool									_initialized = false;

	std::unique_ptr<D_CONTAINERS::DVector<GameObject*>>		GOs = nullptr;

	bool Create(std::string const& name)
	{
		GOs->clear();
		return true;
	}

	GameObject* CreateGameObject()
	{
		GameObject* go = new GameObject();;
		GOs->push_back(go);
		return go;
	}

	D_CONTAINERS::DVector<GameObject*> const* GetGameObjects()
	{
		return GOs.get();
	}

	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;
		GOs = std::make_unique<D_CONTAINERS::DVector<GameObject*>>();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

}