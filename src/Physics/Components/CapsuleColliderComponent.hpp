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

		virtual void						CalculateGeometry(_OUT_ physx::PxGeometry& geom) const override;
		INLINE virtual void					UpdateGeometry() override { CalculateGeometry(mGeometry); }

		virtual D_MATH::Quaternion			GetBiasedRotation() const override;

		static constexpr float				MinRadius = 0.01f;
		static constexpr float				MinHalfHeight = 0.01f;

	protected:

		INLINE virtual physx::PxGeometry const* GetPhysicsGeometry() const override { return &mGeometry; }
		virtual void						CalculateScaledParameters() override;

	private:

		DField(Serialize)
		CapsuleColliderOrientation			mOrientation;

		DField(Serialize)
		float								mRadius;
		DField(Serialize)
		float								mHalfHeight;

		physx::PxCapsuleGeometry			mGeometry;
		float								mScaledRadius;
		float								mScaledHalfHeight;

	public:
		Darius_Physics_CapsuleColliderComponent_GENERATED

	};
}

File_CapsuleColliderComponent_GENERATED
