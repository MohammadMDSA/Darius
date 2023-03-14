#pragma once

#include "Entity.hpp"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#ifndef D_ECS
#define D_ECS Darius::Scene::ECS
#endif // !D_ECS

namespace Darius::Scene::ECS::Components
{
	class ComponentBase;
}

namespace Darius::Scene::ECS
{
	template<class T>
	class CompRef
	{
	public:

		CompRef() = default;

		INLINE				CompRef(Entity ent)
		{
			using conv = std::is_convertible<T*, Darius::Scene::ECS::Components::ComponentBase*>;
			D_STATIC_ASSERT(conv::value);

			if (ent.has<T>())
				mRef = ent.get_ref<T>();
			else
				mRef = flecs::ref<T>();
		}

		INLINE				CompRef(CompRef const& other)
		{
			mRef = other.mRef;
		}

		INLINE T const* Get() const
		{
			if (!IsValid())
				return nullptr;
			return mRef.get();
		}

		INLINE T* Get()
		{
			if (!IsValid())
				return nullptr;
			return mRef.get();
		}

		INLINE bool				IsValid() const
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

		INLINE T const* operator->() const { return Get(); }

		D_CH_R_FIELD(flecs::ref<T>, Ref);
	};
}