#pragma once

#include "Physics/PhysicsScene.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class RigidbodyComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(RigidbodyComponent, ComponentBase, "Physics/Rigidbody", true, false);

	public:
		virtual INLINE bool				IsDisableable() const { return false; }

		// Events
		virtual void					Start() override;
		virtual void					OnDestroy() override;
		virtual void					Update(float) override;
		virtual void					PreUpdate();

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float[]) override;
#endif

		// Serialization
		virtual void					Serialize(D_SERIALIZATION::Json& json) const override;
		virtual void					Deserialize(D_SERIALIZATION::Json const& json) override;

		// Kinematic
		void							SetKinematic(bool value);
		bool							IsKinematic() const;

		void							AddForce(D_MATH::Vector3 const& f);
		void							ClearForce();

		// Gravity
		bool							IsUsingGravity() const;
		void							SetUsingGravity(bool enable);

		// Linear Velocity
		D_MATH::Vector3					GetLinearVelocity() const;
		void							SetLinearVelocity(D_MATH::Vector3 const& v, bool autoWake = true);

		// Angular Velocity
		D_MATH::Vector3					GetAngularVelocity() const;
		void							SetAngularVelocity(D_MATH::Vector3 const& v, bool autoWake = true);

		// Rotation Constraints
		bool							GetRotationConstraintsX() const;
		bool							GetRotationConstraintsY() const;
		bool							GetRotationConstraintsZ() const;
		void							SetRotationConstraintsX(bool enable);
		void							SetRotationConstraintsY(bool enable);
		void							SetRotationConstraintsZ(bool enable);

		// Position Constraints
		bool							GetPositionConstraintsX() const;
		bool							GetPositionConstraintsY() const;
		bool							GetPositionConstraintsZ() const;
		void							SetPositionConstraintsX(bool enable);
		void							SetPositionConstraintsY(bool enable);
		void							SetPositionConstraintsZ(bool enable);

	protected:
		D_CH_FIELD(physx::PxRigidDynamic*, Actor = nullptr);
	private:

#ifdef _D_EDITOR
		bool							DrawRotationConstraints();
		bool							DrawPositionConstraints();
#endif

		bool							mKinematic;
		bool							mUsingGravity;
		bool							mRotationConstraints[3];
		bool							mPositionConstraints[3];

	};
}