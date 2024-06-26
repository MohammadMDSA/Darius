#pragma once

#include "Scene/Scene.hpp"
#include "Scene/GameObject.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/Memory/Allocators/PagedAllocator.hpp>
#include <Utils/Common.hpp>

#include "SceneProxy.generated.hpp"

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{
	template <typename SceneObject>
	class SceneProxy
	{
	public:
		SceneProxy(SceneManager* scene)
		{

		}

		virtual ~SceneProxy()
		{
			for(auto& [k, v] : mGameObjectLookup)
				mSceneAllocator.Free(v);
			mSceneAllocator.Reset();
		}

		INLINE SceneObject*					LookupGameObject(GameObject* go) const
		{
			auto search = mGameObjectLookup.find(go);
			if(search == mGameObjectLookup.end())
				return nullptr;

			return search->second;
		}

	protected:
		SceneObject* AddActor(GameObject* go)
		{
			SceneObject* goProxy = mSceneAllocator.Alloc(std::move(go), std::move(this));
			goProxy->Initialize();
			mGameObjectLookup[go] = goProxy;
			return goProxy;
		}

		virtual bool						RemoveActor(GameObject* go)
		{
			auto search = mGameObjectLookup.find(go);
			if(search == mGameObjectLookup.end())
				return false;

			return RemoveActor(search->second);
		}

		INLINE bool							RemoveActor(SceneObject* proxy)
		{
			D_ASSERT(proxy);

			GameObject* go = proxy->GetGameObject();
			if(!mGameObjectLookup.contains(go))
				return false;
			mGameObjectLookup.erase(go);
			mSceneAllocator.Free(proxy);
			return true;
		}

	private:
		SceneManager*						mScene;
		D_CONTAINERS::DUnorderedMap<GameObject*, SceneObject*> mGameObjectLookup;
		D_MEMORY::PagedAllocator<SceneObject> mSceneAllocator;
	};

	template <typename ObjectType>
	class SceneProxyObjectBase
	{
	public:

		SceneProxyObjectBase(GameObject* go, SceneProxy<ObjectType>* scene) :
			mGameObject(go),
			mSceneProxy(scene)
		{ }

		virtual void Initialize() { }

		virtual ~SceneProxyObjectBase() { }

		INLINE GameObject*					GetGameObject() const { return mGameObject; }
		INLINE SceneProxy<ObjectType>*		GetScene() const { return mSceneProxy; }

	private:
		GameObject*							mGameObject;
		SceneProxy<ObjectType>*				mSceneProxy;
	};
}

File_SceneProxy_GENERATED
