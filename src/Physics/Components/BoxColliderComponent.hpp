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
		D_H_COMP_BODY(BoxColliderComponent, ColliderComponent, "Physics/Box Collider", true);
		
	public:

		virtual void						Awake() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
		virtual void						OnGizmo() const override;
#endif

	protected:

		virtual physx::PxGeometry*			UpdateAndGetPhysicsGeometry(bool& changed);
		INLINE virtual physx::PxGeometry const* GetPhysicsGeometry() const { return &mGeometry; }

	private:
		physx::PxBoxGeometry				mGeometry;


	public:
		Darius_Physics_BoxColliderComponent_GENERATED
	};
}
File_BoxColliderComponent_GENERATED