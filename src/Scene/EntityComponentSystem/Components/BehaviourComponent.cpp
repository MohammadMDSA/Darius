#include "Scene/pch.hpp"
#include "BehaviourComponent.hpp"

using namespace D_LIGHT;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(BehaviourComponent);

	BehaviourComponent::BehaviourComponent() :
		ComponentBase()
	{ }

	BehaviourComponent::BehaviourComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid)
	{ }

}