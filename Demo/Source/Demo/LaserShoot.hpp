#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "LaserShoot.generated.hpp"

namespace Darius::Physics
{
	class ColliderComponent;
	struct HitResult;
}

namespace Demo
{
	class DClass(Serialize) LaserShoot : public D_ECS_COMP::BehaviourComponent
	{
		D_H_BEHAVIOUR_COMP_BODY(LaserShoot, D_ECS_COMP::BehaviourComponent, "Gameplay/Laser Shoot", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

		void							OnHit(Darius::Physics::ColliderComponent* thisCollider, Darius::Physics::ColliderComponent* otherCollider, D_SCENE::GameObject* otherGameObject, Darius::Physics::HitResult const& Hit);

	private:

		DField(Serialize)
		int								mCastType;

	public:
		Demo_LaserShoot_GENERATED;
	};
}

File_LaserShoot_GENERATED
