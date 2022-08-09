#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "Scene.hpp"
#include "GameObject.hpp"

#include <Core/Memory/Memory.hpp>
#include <Renderer/CommandContext.hpp>
#include <Utils/Assert.hpp>

using namespace D_MEMORY;

namespace Darius::Scene
{
	std::unique_ptr<D_CONTAINERS::DVector<GameObject*>>		GOs = nullptr;
	std::unique_ptr<SceneManager>			Manager = nullptr;

	void Update(float deltaTime)
	{
		if (!GOs)
			return;

		Manager->Update(deltaTime);
	}

	void SceneManager::Update(float deltaTime)
	{
		D_GRAPHICS::GraphicsContext& context = D_GRAPHICS::GraphicsContext::Begin(L"Updateing objects");
		
		short index = 0;
		for (auto& go : *GOs.get())
		{
			if (go->GetActive())
				go->Update(context, deltaTime);
			index++;
		}
		context.Finish();
	}

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
		D_ASSERT(!Manager);
		Manager = std::make_unique<SceneManager>();
		GOs = std::make_unique<D_CONTAINERS::DVector<GameObject*>>();
	}

	void Shutdown()
	{
		D_ASSERT(Manager);
		
		Manager.reset();

		for (auto go : *GOs.get())
		{
			delete go;
		}

		GOs.reset();
	}

}