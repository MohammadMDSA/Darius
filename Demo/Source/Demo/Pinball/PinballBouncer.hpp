#pragma once

#include <Physics/Components/RigidbodyComponent.hpp>
#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "PinballBouncer.generated.hpp"

namespace Darius::Physics
{
	class ColliderComponent;
	struct HitResult;
}

namespace Demo
{
	class DClass(Serialize) PinballBouncer : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(PinballBouncer, D_ECS_COMP::BehaviourComponent, "Pinball/PinballBouncer", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;
		virtual void					OnDestroy() override;
		virtual void					Update(float dt) override;

		void							OnColliderEnter(Darius::Physics::ColliderComponent * thisCollider, Darius::Physics::ColliderComponent * otherCollider, D_SCENE::GameObject * otherGameObject, Darius::Physics::HitResult const& Hit);

	private:
		D_CORE::SignalConnection		mColliderConnection;

		DField(Serialize)
		float							mForceAmount = 1.f;

		D_ECS::CompRef<Darius::Physics::RigidbodyComponent> mForceTarget;
		D_MATH::Vector3					mForceDir;
	};
}

File_PinballBouncer_GENERATED
