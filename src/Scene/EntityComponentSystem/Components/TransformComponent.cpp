#include "Scene/pch.hpp"
#include "TransformComponent.hpp"

#include "Scene/Utils/DetailsDrawer.hpp"

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(TransformComponent);

	TransformComponent::TransformComponent() : ComponentBase() {}

	TransformComponent::TransformComponent(D_CORE::Uuid uuid) : ComponentBase(uuid) {}

#ifdef _D_EDITOR
	bool TransformComponent::DrawDetails(float params[])
	{
		auto temp = mTransform;
		if (D_SCENE_DET_DRAW::DrawDetails(temp, nullptr))
		{
			SetTransform(temp);
			return true;
		}
		return false;
	}
#endif
}