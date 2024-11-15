#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "PinballMasterHandle.generated.hpp"

namespace Demo
{
	class DClass(Serialize) PinballMasterHandle : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(PinballMasterHandle, D_ECS_COMP::BehaviourComponent, "Pinball Master/Pinball Master Handle", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;


	private:

		enum class MovementState
		{
			GoingUp,
			GoingDown,
			Up,
			Down
		};

		void							SetGoingUp();
		void							SetGoingDown();

		DField(Serialize)
		float							mMovementTime = 0.5f;

		DField(Serialize)
		float							mRotationAmountDeg = 30.f;

		DField(Serialize)
		D_MATH::Vector3					mAxis = D_MATH::Vector3::Forward;

		DField(Serialize)
		uint8_t							mKeyCode = 95;

		float							mTimeInMovement = 0.f;

		MovementState					mCurrentState = MovementState::Down;

	};
}

File_PinballMasterHandle_GENERATED
