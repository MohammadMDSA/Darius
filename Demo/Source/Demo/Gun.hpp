#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "Gun.generated.hpp"

namespace Demo
{
	class DClass(Serialize) Gun : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(Gun, D_ECS_COMP::BehaviourComponent, "Gameplay/Gun", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

		void							Fire(D_MATH::Vector3 const& direction);

	private:

		DField(Serialize)
		D_SCENE::GameObjectRef			mBulletPrefab;

		DField(Serialize)
		D_SCENE::GameObjectRef			mFlash;

		DField(Serialize)
		D_ECS::CompRef<D_MATH::TransformComponent> mBulletSpawnTransform;

		DField(Serialize)
		float							mFlashTime = 0.1f;

		DField(Serialize)
		float							mBulletSpeed = 20.f;

		float							mFlashStartTime = -1.f;
	};
}

File_Gun_GENERATED
