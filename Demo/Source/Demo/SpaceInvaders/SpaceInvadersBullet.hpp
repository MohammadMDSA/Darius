#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "SpaceInvadersBullet.generated.hpp"

namespace Demo
{
	class DClass(Serialize) SpaceInvadersBullet : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(SpaceInvadersBullet, D_ECS_COMP::BehaviourComponent, "Space Invaders/Space Invaders Bullet", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

		void							SetShooterPlayer(int id);
		INLINE int						GetShooterPlayer() const { return mShooterPlayerId; }


	private:

		DField(Serialize)
		float							mSpeed = 5.f;

		int								mShooterPlayerId = -1;

	};
}

File_SpaceInvadersBullet_GENERATED
