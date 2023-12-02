#pragma once

#include "ColliderComponent.hpp"

#include "MeshColliderComponent.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class DClass(Serialize) MeshColliderComponent : public ColliderComponent
	{
		D_H_COMP_BODY(MeshColliderComponent, ColliderComponent, "Physics/Mesh Collider", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
		virtual void						OnGizmo() const override;
#endif // _D_EDITOR

		virtual void						CalculateGeometry(_OUT_ physx::PxGeometry & geom) const override;
		INLINE virtual void					UpdateGeometry() override { CalculateGeometry(mGeometry); }

		INLINE bool							HasTightBounds() const { return mTightBounds; }
		void								SetTightBounds(bool tight);

	protected:
		INLINE virtual physx::PxGeometry const* GetPhysicsGeometry() const override { return &mGeometry; }

	private:

		DField(Serialize)
		bool										mTightBounds;

		physx::PxConvexMeshGeometry					mGeometry;
		physx::PxConvexMesh*						mMesh;

	public:
		Darius_Physics_MeshColliderComponent_GENERATED
	};
}

File_MeshColliderComponent_GENERATED

