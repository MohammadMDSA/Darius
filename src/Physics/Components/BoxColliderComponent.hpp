#pragma once

#include "ColliderComponent.hpp"

#include "BoxColliderComponent.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class DClass(Serialize) BoxColliderComponent : public ColliderComponent
	{
		GENERATED_BODY();

		D_H_COMP_BODY(BoxColliderComponent, ColliderComponent, "Physics/Box Collider", true);
		
	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
		virtual void						OnGizmo() const override;
#endif

		INLINE D_MATH::Vector3 const&		GetHalfExtents() const { return mHalfExtents; }
		INLINE D_MATH::Vector3 const&		GetScaledHalfExtents() const { return mScaledHalfExtents; }

		void								SetHalfExtents(D_MATH::Vector3 const& halfExtents);
		virtual bool						CalculateGeometry(_OUT_ physx::PxGeometry& geom) const override;
		INLINE virtual bool					UpdateGeometry() override { return CalculateGeometry(mGeometry); }

		static constexpr float				MinExtent = 0.01f;

	protected:

		INLINE virtual physx::PxGeometry const* GetPhysicsGeometry() const override { return &mGeometry; }
		virtual void						CalculateScaledParameters() override;

	private:

		DField(Serialize)
		D_MATH::Vector3						mHalfExtents;

		physx::PxBoxGeometry				mGeometry;
		D_MATH::Vector3						mScaledHalfExtents;


	};
}
File_BoxColliderComponent_GENERATED