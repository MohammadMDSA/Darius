#include "Scene/pch.hpp"
#include "BehaviourComponent.hpp"

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(BehaviourComponent);

	BehaviourComponent::BehaviourComponent() :
		ComponentBase()
	{ }

	BehaviourComponent::BehaviourComponent(D_CORE::Uuid const& uuid) :
		ComponentBase(uuid)
	{ }

}
