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
		GENERATED_BODY();
		D_H_COMP_BODY(SphereColliderComponent, ColliderComponent, "Physics/Sphere Collider", true);
		
	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
		virtual void						OnGizmo() const override;
		virtual void						OnPostComponentAddInEditor() override;
#endif
		virtual bool						CalculateGeometry(_OUT_ physx::PxGeometry& geom) const override;
		INLINE virtual bool					UpdateGeometry() override { return CalculateGeometry(mGeometry); }

		INLINE float						GetRadius() const { return mRadius; }
		INLINE float						GetScaledRadius() const { return mScaledRadius; }
		void								SetRadius(float);

		static constexpr float				MinRadius = 0.01f;

	protected:

		INLINE virtual physx::PxGeometry const* GetPhysicsGeometry() const override { return &mGeometry; }
		virtual void						CalculateScaledParameters() override;


	private:

		DField(Serialize)
		float								mRadius;

		float								mScaledRadius;
		physx::PxSphereGeometry				mGeometry;

	};
}

File_SphereColliderComponent_GENERATED
