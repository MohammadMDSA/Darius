#pragma once

#include "ColliderComponent.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class SphereColliderComponent : public ColliderComponent
	{
		D_H_COMP_BODY(SphereColliderComponent, ColliderComponent, "Physics/Sphere Collider", true, false);
		
	public:

		virtual void						Awake() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		virtual void						Update(float dt) override;
		virtual void						PreUpdate(bool simulating) override;

		D_CH_FIELD(physx::PxSphereGeometry, Geometry);

	protected:

		virtual physx::PxGeometry* UpdateAndGetPhysicsGeometry(bool& changed);
		virtual physx::PxGeometry const* GetPhysicsGeometry() const;

		INLINE float						GetRadius() const
		{
			auto scale = GetTransform().Scale;
			return std::max((float)scale.GetX(), std::max((float)scale.GetY(), (float)scale.GetZ())) / 2;
		}
	};
}
