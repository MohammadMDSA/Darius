#pragma once

#include "Entity.hpp"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

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

			return mEntity.get_mut(mComponent);
		}

		template<class T>
		INLINE T* Get() const
		{
			static ComponentEntry compId = flecs::component<T>();
			D_ASSERT_M(compId == mComponent, "Provided component type as template type is not compatible with the referenced component type");
			if (!IsValid())
				return nullptr;

			return mEntity.get_mut<T>();
		}

		void* operator-> () const { return Get(); }
		bool operator== (UntypedCompRef const& other) const
		{
			return mEntity == other.mEntity && mComponent == other.mComponent;
		}


	private:

		Entity					mEntity;
		ComponentEntry			mComponent;
	};

	template<class T>
	class DClass(Reg) CompRef
	{
		GENERATED_BODY();

	public:

		CompRef() = default;

		INLINE CompRef(Entity ent)
		{
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			using transCheck = std::is_same<T, Darius::Math::TransformComponent>;
			D_STATIC_ASSERT(conv::value || transCheck::value);

			if (ent.has<T>())
				mRef = ent.get_ref<T>();
			else
				mRef = flecs::ref<T>();
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
			return ent.is_valid() && ent.has<T>();
		}

		INLINE T* operator->() { return Get(); }
		INLINE bool operator==(CompRef<T> const& other) const
		{
			if (!mRef.entity().is_valid())
				return false;

			return mRef.entity() == other.mRef.entity();
		}

	private:
		DField(Get[const, &, inline])
		flecs::ref<T>						mRef;
	};

}
File_CompRef_GENERATED