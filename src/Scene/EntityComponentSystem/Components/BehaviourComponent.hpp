#pragma once

#include "ComponentBase.hpp"

#include <Renderer/LightManager.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

#define D_H_BEHAVIOUR_COMP_BODY(type, parent, compName, shouldRegister) D_H_COMP_BODY(type, parent, compName, shouldRegister, true)

namespace Darius::Scene::ECS::Components
{
	class BehaviourComponent : public ComponentBase
	{
		D_H_COMP_BODY(BehaviourComponent, ComponentBase, "Behaviour", false, false);

	};
}