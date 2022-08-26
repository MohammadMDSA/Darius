#include "pch.hpp"
#include <Renderer/pch.hpp>
#include "Scene.hpp"
#include "GameObject.hpp"

#include "EntityComponentSystem/Components/ComponentBase.hpp"
#include "EntityComponentSystem/Components/TransformComponent.hpp"

#include <Core/Memory/Memory.hpp>
#include <Core/Serialization/Json.hpp>
#include <Core/Uuid.hpp>
#include <Renderer/CommandContext.hpp>
#include <Utils/Assert.hpp>

#include <flecs.h>

#include <fstream>

using namespace D_MEMORY;
using namespace D_CORE;

namespace Darius::Scene
{
	std::unique_ptr<D_CONTAINERS::DVector<GameObject*>>					GOs = nullptr;
	std::unique_ptr<D_CONTAINERS::DMap<Uuid, GameObject*, UuidHasher>>	UuidMap = nullptr;

	std::string															SceneName;

	flecs::world														World;

	bool																Loaded = false;

	void SceneManager::Update(float deltaTime)
	{
		if (!GOs)
			return;

		World.progress(deltaTime);

		D_GRAPHICS::GraphicsContext& context = D_GRAPHICS::GraphicsContext::Begin(L"Updateing objects");

		PIXBeginEvent(context.GetCommandList(), PIX_COLOR_DEFAULT, L"Update Mesh Constant Buffers");

		D_CONTAINERS::DVector<GameObject*>::iterator goIterator = GOs->begin();

		while (goIterator != GOs->end())
		{
			if ((*goIterator)->GetActive())
				(*goIterator)->Update(context, deltaTime);
			goIterator++;
		}

		PIXEndEvent(context.GetCommandList());
		context.Finish();
	}

	bool SceneManager::Create(std::string const& name)
	{
		if (Loaded)
			Unload();
		SceneName = name;

#ifdef _DEBUG
		World.set<flecs::Rest>({});
#endif // _DEBUG

		Loaded = true;
		return true;
	}

	GameObject* SceneManager::CreateGameObject()
	{
		auto uuid = GenerateUuid();
		return CreateGameObject(uuid);
	}

	GameObject* SceneManager::CreateGameObject(Uuid uuid)
	{
		// TODO: Better allocation
		auto go = new GameObject(uuid, World.entity(("GameObject:" + ToString(uuid)).c_str()));
		GOs->push_back(go);

		// Update uuid map
		UuidMap->emplace(uuid, go);

		return go;
	}

	// Slow function
	void SceneManager::GetGameObjects(D_CONTAINERS::DVector<GameObject*>& container)
	{
		for (auto& go : *GOs)
		{
			container.push_back(go);
		}
	}

	void SceneManager::Initialize()
	{
		D_ASSERT(!GOs);
		UuidMap = std::make_unique<D_CONTAINERS::DMap<Uuid, GameObject*, UuidHasher>>();
		GOs = std::make_unique<D_CONTAINERS::DVector<GameObject*>>();
	}

	void SceneManager::Shutdown()
	{
		D_ASSERT(GOs);

		GOs.reset();
		UuidMap.reset();
	}

	bool SceneManager::Load(std::wstring const& path)
	{
		D_SERIALIZATION::Json sceneJson;
		auto ifs = std::ifstream(path);

		if (!ifs)
			return false;

		ifs >> sceneJson;
		ifs.close();

		auto filePath = Path(path);
		auto filename = filePath.string();
		filename = filename.substr(0, filename.size() - filePath.extension().string().size());

		Create(filename);

		for (int i = 0; i < sceneJson["Objects"].size(); i++)
		{
			D_SERIALIZATION::Json jObj = sceneJson["Objects"][i];

			Uuid uuid;
			D_CORE::from_json(jObj["Uuid"], uuid);
			auto obj = CreateGameObject(uuid);

			from_json(jObj, *obj);
		}
	}

	bool SceneManager::Save(std::string const& name, const Path& path)
	{
		D_SERIALIZATION::Json sceneJson = D_SERIALIZATION::Json::object();

		DVector<GameObject> rawGos;
		for (auto go : *GOs)
		{
			rawGos.push_back(*go);
		}

		sceneJson["Objects"] = rawGos;

		auto pathName = Path(path).append(name).string() + ".dar";

		auto ofs = std::ofstream(pathName);
		if (!ofs)
			return false;

		ofs << sceneJson;

		return true;
	}

	void SceneManager::Unload()
	{
		for (auto& go : *GOs)
		{
			World.remove_all(go->GetEntity());
			delete go;
		}
		GOs->clear();
		UuidMap->clear();
	}

	D_ECS::ECSRegistry& SceneManager::GetRegistry()
	{
		return World;
	}

}