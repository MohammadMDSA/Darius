#pragma once

#include "Physics/PhysicsScene.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "RigidbodyComponent.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	enum class DEnum(Serialize) ForceMode
	{
		Force,				//!< parameter has unit of mass * length / time^2, i.e., a force
		Impulse,			//!< parameter has unit of mass * length / time, i.e., force * time
		VelocityChange,		//!< parameter has unit of length / time, i.e., the effect is mass independent: a velocity change.
		Acceleration		//!< parameter has unit of length/ time^2, i.e., an acceleration. It gets treated just like a force except the mass is not divided out before integration.
	};

	class DClass(Serialize[bKinematic, bUsingGravity, RotationConstraintsX, RotationConstraintsY, RotationConstraintsZ, PositionConstraintsX, PositionConstraintsY, PositionConstraintsZ, CenterOfMass, Mass]) RigidbodyComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(RigidbodyComponent, ComponentBase, "Physics/Rigidbody", true);
		GENERATED_BODY();
	public:
		virtual INLINE bool				IsDisableable() const { return false; }

		// Events
		virtual void					Awake() override;
		virtual void					Start() override;
		virtual void					OnDestroy() override;
		virtual void					OnActivate() override;
		virtual void					OnDeactivate() override;

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float[]) override;
		bool							DrawRotationConstraints();
		bool							DrawPositionConstraints();
		virtual void					OnPostComponentAddInEditor() override;
#endif

		// Kinematic
		void							SetKinematic(bool value);
		bool							IsKinematic() const;

		void							AddForce(D_MATH::Vector3 const& f, ForceMode mode = ForceMode::Force);
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

		D_MATH::Vector3					GetCenterOfMass() const;
		void							SetCenterOfMass(D_MATH::Vector3 const& c);

		INLINE float					GetMass() const { return mMass; }
		void							SetMass(float mass);

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

	private:

		// Internals
		D_CORE::Ref<PhysicsActor>		mActor = nullptr;
		D_MATH::Vector3					mCenterOfMass;
		float							mMass;
		uint8_t							mKinematic : 1;
		uint8_t							mUsingGravity : 1;
		uint8_t							mRotationConstraintsX : 1;
		uint8_t							mRotationConstraintsY : 1;
		uint8_t							mRotationConstraintsZ : 1;
		uint8_t							mPositionConstraintsX : 1;
		uint8_t							mPositionConstraintsY : 1;
		uint8_t							mPositionConstraintsZ : 1;
		

	};
}

File_RigidbodyComponent_GENERATED
