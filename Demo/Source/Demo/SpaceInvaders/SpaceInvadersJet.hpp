#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "SpaceInvadersJet.generated.hpp"

namespace Darius::Physics
{
	class ColliderComponent;
}

namespace Demo
{
	class DClass(Serialize) SpaceInvadersJet : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(SpaceInvadersJet, D_ECS_COMP::BehaviourComponent, "Space Invaders/Space Invaders Jet", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;
		virtual void					OnDestroy() override;

		virtual void					Update(float deltaTime) override;

		void							Fire();

		void							Move(D_MATH::Vector3 const& normalizedDir, float dt);

		void							PlayerControl(float dt);
		void							AiControl(float dt);

		INLINE int						GetPlayerId() const { return mIsPlayer ? 0 : 1; }

	private:

		void							OnTrigger(Darius::Physics::ColliderComponent * thisCollider, D_SCENE::GameObject * otherGameObject);

		DField(Serialize)
		bool										mIsPlayer = false;

		DField(Serialize)
		D_SCENE::GameObjectRef						mBulletPrefab;

		DField(Serialize)
		D_ECS::CompRef<D_MATH::TransformComponent>	mBulletSpawnTransform;

		DField(Serialize)
		float										mSpeed = 1.f;

		DField(Serialize)
		float										mAiDirectionMovementTime = 1.f;

		DField(Serialize)
		float										mAiFireCooldown = 0.5f;

		float										mAiCurrentDirectionMovementTime = 0.f;
		float										mAiLastFireTime = 0.f;
		bool										mAiMovingRight = true;

		D_CORE::SignalConnection					mTriggerSignal;
	};
}

File_SpaceInvadersJet_GENERATED
