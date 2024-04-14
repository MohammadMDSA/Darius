#pragma once

#include "Scene.hpp"

#include <Core/RefCounting/Ref.hpp>
#include <Core/Serialization/TypeSerializer.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{
	class GameObject;

	class GameObjectRef : public D_CORE::Ref<GameObject>
	{
	public:

		GameObjectRef() : D_CORE::Ref<GameObject>() { }

		template<class OTHER>
		GameObjectRef(D_CORE::Ref<OTHER> const& other) : D_CORE::Ref<GameObject>(other) { }

		GameObjectRef(GameObject* gameObject) : D_CORE::Ref<GameObject>(gameObject) { }

		GameObjectRef(GameObjectRef const& other) : D_CORE::Ref<GameObject>(other) { }

		GameObjectRef& operator= (GameObjectRef const&) = default;

		D_CORE::Uuid GetUuid() const;

		virtual bool IsValid() const;

		bool IsValid(_OUT_ bool& isMissing) const;

	};
}

namespace rttr
{
	template<>
	struct wrapper_mapper<D_SCENE::GameObjectRef>
	{
		using wrapped_type = D_SERIALIZATION::UuidWrapper;
		using type = D_SCENE::GameObjectRef;

		INLINE static wrapped_type get(type const& obj)
		{
			if (!obj.IsValid())
				return { D_CORE::Uuid(), D_SERIALIZATION_UUID_PARAM_GAMEOBJECT };
			return { obj.GetUuid(), D_SERIALIZATION_UUID_PARAM_GAMEOBJECT };
		}

		static INLINE type create(wrapped_type const& value)
		{
			return D_SCENE::GameObjectRef(D_WORLD::GetGameObject(value.Uuid));
		}
	};
}
