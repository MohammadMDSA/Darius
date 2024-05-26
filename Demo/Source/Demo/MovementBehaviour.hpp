#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "MovementBehaviour.generated.hpp"

namespace Demo
{
	
	class DClass(Serialize) MovementBehaviour : public D_ECS_COMP::BehaviourComponent
	{
		D_H_BEHAVIOUR_COMP_BODY(MovementBehaviour, D_ECS_COMP::BehaviourComponent, "Gameplay/Movement", true, true);
		GENERATED_BODY();

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

	private:

		DField(Get, Set)
		bool mTRotate;

		DField(Get[inline], Set[inline])
		bool							mRotate;

		DField(Serialize)
		D_MATH::Vector3					mAxis;

		DField(Serialize)
		float							mRange;
	};
}

File_MovementBehaviour_GENERATED