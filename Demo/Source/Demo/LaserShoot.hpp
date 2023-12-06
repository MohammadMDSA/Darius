#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "LaserShoot.generated.hpp"

namespace Demo
{
	class DClass(Serialize) LaserShoot : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(LaserShoot, D_ECS_COMP::BehaviourComponent, "Gameplay/Laser Shoot", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Update(float deltaTime) override;

	private:

		DField(Serialize)
		int								mCastType;

	};
}

File_LaserShoot_GENERATED
