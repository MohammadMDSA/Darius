#pragma once

#include "ColliderComponent.hpp"

#include "SphereColliderComponent.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class DClass(Serialize) SphereColliderComponent : public ColliderComponent
	{
		D_H_COMP_BODY(SphereColliderComponent, ColliderComponent, "Physics/Sphere Collider", true);
		
	public:

		virtual void						Awake() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
		virtual void						OnGizmo() const override;

#endif

		static constexpr float				MinRadius = 0.01f;


	protected:

		virtual physx::PxGeometry*			UpdateAndGetPhysicsGeometry(bool& changed) override;
		INLINE virtual physx::PxGeometry const* GetPhysicsGeometry() const override { return &mGeometry; }
		virtual void						CalculateGeometry(_OUT_ physx::PxGeometry & geom) const override;
		virtual void						CalculateScaledParameters() override;

		INLINE float						GetRadius() const { return mRadius; }
		INLINE float						GetScaledRadius() const { return mScaledRadius; }
		void								SetRadius(float);

	private:

		DField(Serialize)
		float								mRadius;

		float								mScaledRadius;
		physx::PxSphereGeometry				mGeometry;

	public:
		Darius_Physics_SphereColliderComponent_GENERATED
	};
}

File_SphereColliderComponent_GENERATED
