#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "CollisionTest.generated.hpp"


namespace Darius::Physics
{
	class ColliderComponent;
	struct HitResult;
}


namespace Demo
{
	class DClass(Serialize) CollisionTest : public D_ECS_COMP::BehaviourComponent
	{
		D_H_BEHAVIOUR_COMP_BODY(CollisionTest, D_ECS_COMP::BehaviourComponent, "Physics/Debug/Collision Test", true, true);

		GENERATED_BODY();

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

	private:
		void	OnTouchEnter(Darius::Physics::ColliderComponent * thisCollider, Darius::Physics::ColliderComponent * otherCollider, D_SCENE::GameObject * otherGameObject, Darius::Physics::HitResult const& Hit);
		void	OnTouchStay(Darius::Physics::ColliderComponent * thisCollider, Darius::Physics::ColliderComponent * otherCollider, D_SCENE::GameObject * otherGameObject, Darius::Physics::HitResult const& Hit);
		void	OnTouchExit(Darius::Physics::ColliderComponent* thisCollider, Darius::Physics::ColliderComponent* otherCollider, D_SCENE::GameObject* otherGameObject, Darius::Physics::HitResult const& Hit);
	};
}

File_CollisionTest_GENERATED
