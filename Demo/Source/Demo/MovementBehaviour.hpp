#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>
#include <Renderer/Light/LightManager.hpp>

#include "MovementBehaviour.generated.hpp"

namespace Demo
{
	
	class DClass(Serialize) MovementBehaviour : public D_ECS_COMP::BehaviourComponent
	{
		D_H_BEHAVIOUR_COMP_BODY(MovementBehaviour, D_ECS_COMP::BehaviourComponent, "Movement", true, true);

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

		D_CH_RW_FIELD(bool, Rotate);

		Demo_MovementBehaviour_GENERATED;
	};
}

File_MovementBehaviour_GENERATED