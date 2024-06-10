#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>
#include <Scene/EntityComponentSystem/CompRef.hpp>

#include "BulletShooter.generated.hpp"

namespace Demo
{
	class DClass(Serialize) BulletShooter : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(BulletShooter, D_ECS_COMP::BehaviourComponent, "Gameplay/Bullet Shooter", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		virtual void					Update(float deltaTime) override;

	private:

		DField(Serialize)
		D_ECS::CompRef<class Gun>		mGun;

		DField(Serialize)
		D_ECS::CompRef<D_MATH::TransformComponent> mRefAim;
	};
}

File_BulletShooter_GENERATED
