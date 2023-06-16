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

	protected:

		virtual physx::PxGeometry* UpdateAndGetPhysicsGeometry(bool& changed);
		virtual physx::PxGeometry const* GetPhysicsGeometry() const;

		INLINE float						GetRadius() const
		{
			auto scale = GetTransform()->GetScale();
			return std::max((float)scale.GetX(), std::max((float)scale.GetY(), (float)scale.GetZ())) / 2;
		}

	private:
		physx::PxSphereGeometry				mGeometry;

	public:
		Darius_Physics_SphereColliderComponent_GENERATED
	};
}

File_SphereColliderComponent_GENERATED
