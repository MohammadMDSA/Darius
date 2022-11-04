#pragma once

#include "ColliderComponent.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class BoxColliderComponent : public ColliderComponent
	{
		D_H_COMP_BODY(BoxColliderComponent, ColliderComponent, "Physics/Box Collider", true, false);
		
	public:

		virtual void						Start() override;
		
		virtual void						Update(float dt) override;
		virtual void						PreUpdate(bool simulating) override;

		D_CH_FIELD(physx::PxBoxGeometry, Geometry);

	protected:

		virtual physx::PxGeometry* UpdateAndGetPhysicsGeometry(bool& changed);
		virtual physx::PxGeometry const* GetPhysicsGeometry() const;
	};
}
