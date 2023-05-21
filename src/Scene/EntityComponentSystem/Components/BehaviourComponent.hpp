#pragma once

#include "ComponentBase.hpp"
#include "Scene/GameObjectRef.hpp"

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

#define D_H_BEHAVIOUR_COMP_BODY(type, parent, compName, shouldRegister, receivesUpdates) D_H_COMP_BODY_RAW(type, parent, compName, shouldRegister, true, receivesUpdates) \
static void ComponentUpdater(float dt, D_ECS::ECSRegistry& reg); \
static void ComponentLateUpdater(float dt, D_ECS::ECSRegistry& reg)

#define D_H_BEHAVIOUR_COMP_DEF(type) D_H_COMP_DEF(type) \
void type::ComponentUpdater(float dt, D_ECS::ECSRegistry& reg) \
{ \
	reg.each([&](type& comp) \
		{ \
			if(comp.IsActive()) \
				comp.Update(dt); \
		} \
	); \
} \
void type::ComponentLateUpdater(float dt, D_ECS::ECSRegistry& reg) \
{ \
	reg.each([&](type& comp) \
		{ \
			if(comp.IsActive()) \
				comp.LateUpdate(dt); \
		} \
	); \
}

#define D_H

namespace Darius::Scene::ECS::Components
{
	class BehaviourComponent : public ComponentBase
	{
		D_H_COMP_BODY(BehaviourComponent, ComponentBase, "Behaviour", false);

	};
}