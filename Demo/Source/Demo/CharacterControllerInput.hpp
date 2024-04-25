#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include <Physics/Components/CharacterControllerComponent.hpp>

#include "CharacterControllerInput.generated.hpp"

namespace Demo
{
	class DClass(Serialize) CharacterControllerInput : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(CharacterControllerInput, D_ECS_COMP::BehaviourComponent, "Gameplay/Character Controller Input", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;
		virtual void					OnDestroy() override;

		virtual void					Update(float deltaTime) override;

	private:

		DField(Serialize)
		float							mSpeed;

		DField(Serialize)
		float							mMouseSpeed;

		DField(Serialize)
		float							mJumpSpeed;

		D_ECS::CompRef<D_PHYSICS::CharacterControllerComponent> mController;

	};
}

File_CharacterControllerInput_GENERATED
