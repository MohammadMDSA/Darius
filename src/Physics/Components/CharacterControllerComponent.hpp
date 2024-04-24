#pragma once

#include "Physics/Resources/PhysicsMaterialResource.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>
#include <ResourceManager/ResourceRef.hpp>

#include <characterkinematic/PxCapsuleController.h>

#include <CharacterControllerComponent.generated.hpp>

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class DClass(Serialize) CharacterControllerComponent : public D_ECS_COMP::ComponentBase
	{
		GENERATED_BODY();
		D_H_COMP_BODY(CharacterControllerComponent, D_ECS_COMP::ComponentBase, "Physics/Character Controller", true);


	public:
		enum class DEnum(Serialize) ClimbingMode
		{
			Easy,
			Constrained
		};

		enum class DEnum(Serialize) NonWalkableMode
		{
			PreventClimbing,
			PreventClimbingAndForceSliding
		};

	public:
		void									Awake() override;
		void									OnDestroy() override;
		void									Update(float dt) override;
		void									PreUpdate();
		void									Move(D_MATH::Vector3 const& displacement);

#if _D_EDITOR
		virtual bool							DrawDetails(float params[]) override;
		virtual void							OnGizmo() const override;
#endif // _D_EDITOR

		// Height
		INLINE float							GetHeight() const { return mHeight; }
		void									SetHeight(float height, bool preserveBottom = true);

		// Radius
		INLINE float							GetRadius() const { return mRadius; }
		void									SetRadius(float value);

		// Climbing Mode
		INLINE ClimbingMode						GetClimbingMode() const { return mClimbingMode; }
		void									SetClimbingMode(ClimbingMode climbingMode);

		// Nonwalkable Mode
		INLINE NonWalkableMode					GetNonWalkableMode() const { return mNonWalkableMode; }
		void									SetNonWalkableMode(NonWalkableMode mode);

		// Slope Limit
		INLINE float							GetSlopeLimit() const { return mSlopeLimit; }
		// A degree in [0, 90] is spected
		void									SetSlopeLimit(float deg);

		// Contact Offset
		INLINE float							GetContactOffset() const { return mContactOffset; }
		// Should be a small positive non zero value
		void									SetContactOffset(float offset);

		// Step Offset
		INLINE float							GetStepOffset() const { return mStepOffset; }
		void									SetStepOffset(float value);

		// Up Direction
		INLINE D_MATH::Vector3 const&			GetUpDirection() const { return mUpDirection; }
		void									SetUpDirection(D_MATH::Vector3 const& upVec);

		// Gravity Scale
		INLINE float							GetGravityScale() const { return mGravityScale; }
		void									SetGravityScale(float scale);
		// Material
		INLINE PhysicsMaterialResource*			GetPhysicsMaterial() const { return mPhysicsMaterial.Get(); }
		void									SetPhysicsMaterial(PhysicsMaterialResource* material);

		INLINE D_MATH::Vector3					GetScaledGravity() const { D_ASSERT(mScene); return mScene->GetGravityVector() * mGravityScale; }

	private:

		DField(Serialize)
		float									mHeight;

		DField(Serialize)
		float									mRadius;

		DField(Serialize)
		ClimbingMode							mClimbingMode;

		DField(Serialize)
		NonWalkableMode							mNonWalkableMode;

		DField(Serialize)
		float									mSlopeLimit;

		DField(Serialize)
		float									mContactOffset;

		DField(Serialize)
		float									mStepOffset;

		DField(Serialize)
		D_MATH::Vector3							mUpDirection;

		DField(Serialize)
		float									mGravityScale;

		DField(Serialize)
		D_RESOURCE::ResourceRef<PhysicsMaterialResource> mPhysicsMaterial;

		// Internal
		physx::PxCapsuleController*				mController;
		D_PHYSICS::PhysicsScene*				mScene;
		D_MATH::Vector3							mMovement;
		float									mFallSpeed;
	};
}
