#pragma once

#include "ColliderComponent.hpp"

#include "CapsuleColliderComponent.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class DClass(Serialize) CapsuleColliderComponent : public ColliderComponent
	{
		D_H_COMP_BODY(CapsuleColliderComponent, ColliderComponent, "Physics/Capsule Collider", true);
	public:
		enum class DEnum(Serialize) CapsuleColliderOrientation
		{
			AlongX = 0,
			AlongY = 1,
			AlongZ = 2
		};

	public:

		virtual void						Awake() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
		virtual void						OnGizmo() const override;
#endif

		INLINE float						GetRadius() const { return mRadius; }
		INLINE float						GetHalfHeight() const { return mHalfHeight; }
		INLINE CapsuleColliderOrientation	GetOrientation() const { return mOrientation; }

		void								SetOrientation(CapsuleColliderOrientation orientation);
		void								SetRadius(float radius);
		void								SetHalfHeight(float halfHeight);

		// Call when are the parameters are correctly set
		void								CalculateGeometry(_OUT_ physx::PxGeometry& geom) const;

		static constexpr float				MinRadius = 0.01f;
		static constexpr float				MinHalfHeight = 0.01f;

	protected:

		virtual physx::PxGeometry*			UpdateAndGetPhysicsGeometry(bool& changed) override;
		INLINE virtual physx::PxGeometry const* GetPhysicsGeometry() const override { return &mGeometry; }

	private:
		void								CalculateScaledParameters();

		DField(Serialize)
		CapsuleColliderOrientation			mOrientation;

		DField(Serialize)
		float								mRadius;
		DField(Serialize)
		float								mHalfHeight;

		physx::PxCapsuleGeometry			mGeometry;
		D_MATH::Vector3						mUsedScale;
		float								mScaledRadius;
		float								mScaledHalfHeight;

	public:
		Darius_Physics_CapsuleColliderComponent_GENERATED

	};
}

File_CapsuleColliderComponent_GENERATED
