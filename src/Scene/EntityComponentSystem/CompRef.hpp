#pragma once

#include "Entity.hpp"

#include "Scene/GameObjectRef.hpp"
#include "Scene/GameObject.hpp"
#include "Scene/Scene.hpp"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#include <sstream>

#ifndef D_ECS
#define D_ECS Darius::Scene::ECS
#endif // !D_ECS

#include "CompRef.generated.hpp"

namespace Darius::Scene::ECS::Components
{
	class ComponentBase;
}

namespace Darius::Math
{
	class TransformComponent;
}

namespace Darius::Scene::ECS
{
	class DClass(Reg) UntypedCompRef
	{
		GENERATED_BODY();
	public:

		UntypedCompRef() = default;

		INLINE UntypedCompRef(Entity ent, ComponentEntry component) :
			mEntity(ent),
			mComponent(component)
		{
		}

		INLINE UntypedCompRef(UntypedCompRef const& other) :
			mEntity(other.mEntity),
			mComponent(other.mComponent)
		{

		}

		INLINE bool IsValid() const
		{
			return mEntity.is_valid() && mEntity.has(mComponent);
		}

		INLINE void* Get() const
		{
			if (!IsValid())
				return nullptr;

			return const_cast<void*>(mEntity.get(mComponent));
		}

		template<class T>
		INLINE T* Get() const
		{
			static ComponentEntry compId = flecs::component<T>();
			D_ASSERT_M(compId == mComponent, "Provided component type as template type is not compatible with the referenced component type");
			if (!IsValid())
				return nullptr;

			return const_cast<T*>(mEntity.get<T>());
		}

		void* operator-> () const { return Get(); }
		bool operator== (UntypedCompRef const& other) const
		{
			return mEntity == other.mEntity && mComponent == other.mComponent;
		}


	private:

		friend struct std::hash<UntypedCompRef>;

		Entity					mEntity;
		ComponentEntry			mComponent;
	};

	template<class T>
	class DClass(Reg) CompRef
	{
		GENERATED_BODY();

	public:

		CompRef() = default;

		CompRef(T * comp)
		{
			if (!comp)
			{
				mRef = flecs::ref<T>();
				return;
			}

			auto go = comp->GetGameObject();
			D_ASSERT(go);
			auto ent = go->GetEntity();
			mRef = ent.get_ref<T>();
		}

		explicit CompRef(GameObject const* go)
		{
			if(!go)
				mRef = flecs::ref<T>();

			auto ent = go->GetEntity();
			mRef = ent.get_ref<T>();
		}

		INLINE explicit CompRef(Entity ent)
		{
			mRef = ent.get_ref<T>();
		}

		INLINE CompRef(CompRef const& other)
		{
			mRef = other.mRef;
		}

		INLINE T* Get()
		{
			if (!IsValid())
				return nullptr;
			return mRef.get();
		}

		INLINE bool IsValid() const
		{
			auto ent = mRef.entity();
			auto go = D_WORLD::GetGameObject(ent);
			return go && go->IsValid() && go->HasComponent<T>();
		}

		INLINE bool IsNull() const
		{
			return mRef.entity().id() == 0u;
		}

		// When it is not null but still not valid
		INLINE bool IsMissing() const
		{
			return !IsNull() && !IsValid();
		}

		INLINE Entity GetEntity() const
		{
			return mRef.entity();
		}

		INLINE GameObject* GetGameObject() const
		{
			return D_WORLD::GetGameObject(GetEntity());
		}

		INLINE T* operator->() { return Get(); }
		INLINE bool operator==(CompRef<T> const& other) const
		{
			if (!mRef.entity().is_valid())
				return false;

			return mRef.entity() == other.mRef.entity();
		}

		INLINE bool operator==(T const* other) const
		{
			bool null = IsNull();
			if (null && other == nullptr)
				return true;
			else if (null || other == nullptr)
				return false;
			else
				return mRef.entity() == other->GetGameObject()->GetEntity();
		}

	private:
		DField(Get[const, &, inline])
			flecs::ref<T>						mRef;
	};
}

namespace std
{
	template<>
	struct hash<D_ECS::UntypedCompRef>
	{
		INLINE size_t operator() (D_ECS::UntypedCompRef const& ref)
		{
			hash<flecs::id_t> hasher;
			return hasher(ref.mEntity) + hasher(ref.mComponent);
		}
	};
}


File_CompRef_GENERATED