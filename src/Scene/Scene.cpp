#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "Scene.hpp"
#include "GameObject.hpp"

#include <Core/Memory/Memory.hpp>
#include <Renderer/CommandContext.hpp>
#include <Utils/Assert.hpp>

using namespace D_MEMORY;
using namespace D_CORE;

namespace Darius::Scene
{
	std::unique_ptr<D_CONTAINERS::DVector<GameObject*>>		GOs = nullptr;

	void SceneManager::Update(float deltaTime)
	{
		if (!GOs)
			return;

		D_GRAPHICS::GraphicsContext& context = D_GRAPHICS::GraphicsContext::Begin(L"Updateing objects");
		
		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, L"Update Mesh Constant Buffers");

		short index = 0;
		for (auto& go : *GOs.get())
		{
			if (go->GetActive())
				go->Update(context, deltaTime);
			index++;
		}

		PIXEndEvent(context.GetCommandList());
		context.Finish();
	}

	bool SceneManager::Create(std::string const& name)
	{
		GOs->clear();
		return true;
	}

	GameObject* SceneManager::CreateGameObject()
	{
		GameObject* go = new GameObject(GenerateUuid());
		GOs->push_back(go);
		return go;
	}

	D_CONTAINERS::DVector<GameObject*> const* SceneManager::GetGameObjects()
	{
		return GOs.get();
	}

	void SceneManager::Initialize()
	{
		D_ASSERT(!GOs);
		GOs = std::make_unique<D_CONTAINERS::DVector<GameObject*>>();
	}

	void SceneManager::Shutdown()
	{
		D_ASSERT(GOs);
		

		for (auto go : *GOs.get())
		{
			delete go;
		}

		GOs.reset();
	}

}