#pragma once

#include "GameObject.hpp"
#include "Scene.hpp"

#include <Core/Ref.hpp>

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{
	class GameObjectRef : public D_CORE::Ref<GameObject>
	{
	public:

		GameObjectRef() : GameObjectRef(nullptr, std::nullopt) { }

		GameObjectRef(D_CORE::CountedOwner ownerData) : GameObjectRef(nullptr, ownerData) { }

		GameObjectRef(GameObject* data, std::optional<D_CORE::CountedOwner> ownerData = std::nullopt) :
			D_CORE::Ref<GameObject>(data, ownerData)
		{
			if (data)
				mUuid = data->GetUuid();
			else
				mUuid = D_CORE::Uuid();
		}

		INLINE virtual bool IsValid() const override
		{
			auto sceneObj = D_WORLD::GetGameObject(mUuid);
			return D_CORE::Ref<GameObject>::IsValid() && sceneObj && sceneObj->IsValid();
		}

		INLINE virtual void SetData(GameObject* data) override
		{
			D_CORE::Ref<GameObject>::SetData(data);
			if (data)
				mUuid = data->GetUuid();
		}

	private:
		D_CORE::Uuid			mUuid;
	};
}

namespace rttr
{
	template<>
	struct wrapper_mapper<D_SCENE::GameObjectRef>
	{
		using wrapped_type = D_CORE::Uuid;
		using type = D_SCENE::GameObjectRef;

		INLINE static wrapped_type get(type const& obj)
		{
			if (!obj.IsValid())
				return D_CORE::Uuid();
			return obj->GetUuid();
		}

		static INLINE type create(wrapped_type const& value)
		{
			return D_SCENE::GameObjectRef(D_WORLD::GetGameObject(value));
		}
	};
}
