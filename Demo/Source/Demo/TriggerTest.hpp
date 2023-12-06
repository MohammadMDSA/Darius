#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "TriggerTest.generated.hpp"

namespace Darius::Physics
{
	class ColliderComponent;
}

namespace Demo
{
	class DClass(Serialize) TriggerTest : public D_ECS_COMP::BehaviourComponent
	{
		D_H_BEHAVIOUR_COMP_BODY(TriggerTest, D_ECS_COMP::BehaviourComponent, "Physics/Debug/Trigger Test", true, true);
		GENERATED_BODY();

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

	private:
		void OnTriggerEnter(Darius::Physics::ColliderComponent * thisCollider, D_SCENE::GameObject * otherGameObject);
		void OnTriggerExit(Darius::Physics::ColliderComponent* thisCollider, D_SCENE::GameObject* otherGameObject);

	};
}

File_TriggerTest_GENERATED
