#include "Scene/pch.hpp"
#include "TransformComponent.hpp"

namespace Darius::Scene::ECS::Components
{
	TransformComponent::TransformComponent() : ComponentBase() {}

	TransformComponent::TransformComponent(D_CORE::Uuid uuid) : ComponentBase(uuid) {}

	D_H_COMP_DEF(TransformComponent);
}