#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "RotatingMovementComponent.generated.hpp"

namespace Demo
{
	class DClass(Serialize) RotatingMovementComponent : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(RotatingMovementComponent, D_ECS_COMP::BehaviourComponent, "Utils/Rotating Movement", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR


		virtual void					Update(float deltaTime) override;

		// Local
		INLINE bool						IsLocal() const { return mLocal; }
		void							SetLocal(bool local);

		// Speed
		INLINE float					GetSpeed() const { return mSpeed; }
		void							SetSpeed(float speed);

		// Axis
		INLINE D_MATH::Vector3 const&	GetAxis() const { return mAxis; }
		void							SetAxis(D_MATH::Vector3 const& axis);

	private:

		DField(Serialize)
		float							mSpeed;

		DField(Serialize)
		D_MATH::Vector3					mAxis;

		DField(Serialize)
		bool							mLocal;
	};
}

File_Rotator_GENERATED
