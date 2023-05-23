#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "GameObjectReferencer.generated.hpp"

namespace Demo
{
	class DClass(Serialize) GameObjectReferencer : public D_ECS_COMP::BehaviourComponent
	{
		D_H_BEHAVIOUR_COMP_BODY(GameObjectReferencer, D_ECS_COMP::BehaviourComponent, "Utils/Game Object Referencer", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

		void							SetReference(D_SCENE::GameObject* go);


	private:

		DField(Serialize)
		D_SCENE::GameObjectRef			mReference;

	public:
		Demo_GameObjectReferencer_GENERATED
	};
}

File_GameObjectReferencer_GENERATED
