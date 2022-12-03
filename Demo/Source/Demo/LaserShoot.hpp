#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

namespace Demo
{
	class LaserShoot : public D_ECS_COMP::BehaviourComponent
	{
		D_H_BEHAVIOUR_COMP_BODY(LaserShoot, D_ECS_COMP::BehaviourComponent, "Gameplay/Laser Shoot", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;
	};
}