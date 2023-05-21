#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "Targeter.generated.hpp"

namespace Demo
{
	class DClass(Serialize) Targeter : public D_ECS_COMP::BehaviourComponent
	{
		D_H_BEHAVIOUR_COMP_BODY(Targeter, D_ECS_COMP::BehaviourComponent, "Targeter", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

		void							SetTarget(D_SCENE::GameObject* go);

	private:
		DField(Serialize)
		D_SCENE::GameObjectRef			mTargetObject;

	public:
		Demo_Targeter_GENERATED
	};
}

File_Targeter_GENERATED
