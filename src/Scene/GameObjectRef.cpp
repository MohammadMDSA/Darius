#include "Scene/pch.hpp"
#include "GameObjectRef.hpp"

#include "Scene.hpp"
#include "GameObject.hpp"

namespace Darius::Scene
{
	D_CORE::Uuid GameObjectRef::GetUuid() const
	{
		return Get()->GetUuid();
	}

	bool GameObjectRef::IsValid(_OUT_ bool& isMissing) const
	{
		if (IsNull())
		{
			isMissing = false;
			return false;
		}

		auto sceneObj = D_WORLD::GetGameObject(Get()->GetEntity());
		isMissing = sceneObj == nullptr;
		return D_CORE::Ref<GameObject>::IsValid();
	}

	bool GameObjectRef::IsValid() const
	{
		if (!D_CORE::Ref<GameObject>::IsValid())
			return false;
		auto sceneObj = D_WORLD::GetGameObject(Get()->GetEntity());
		return sceneObj && sceneObj->IsValid();
	}
}
