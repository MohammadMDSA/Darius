#pragma once

#include "ColliderComponent.hpp"

#include <Graphics/GraphicsUtils/Buffers/ReadbackBuffer.hpp>
#include <Renderer/Resources/StaticMeshResource.hpp>

#include "MeshColliderComponent.generated.hpp"

#ifndef D_PHYSICS
#define D_PHYSICS Darius::Physics
#endif // !D_PHYSICS

namespace Darius::Physics
{
	class DClass(Serialize) MeshColliderComponent : public ColliderComponent
	{
		GENERATED_BODY();
		D_H_COMP_BODY(MeshColliderComponent, ColliderComponent, "Physics/Mesh Collider", true);

	public:

		void								Awake() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
		virtual void						OnGizmo() const override;
#endif // _D_EDITOR

		virtual bool						CalculateGeometry(_OUT_ physx::PxGeometry & geom) const override;
		INLINE virtual bool					UpdateGeometry() override { return CalculateGeometry(mGeometry); }

		INLINE bool							HasTightBounds() const { return mTightBounds; }
		void								SetTightBounds(bool tight);

		D_RENDERER::StaticMeshResource*		GetReferenceMesh() const { return mReferenceMesh.Get(); }
		void								SetReferenceMesh(D_RENDERER::StaticMeshResource* staticMesh);

	protected:
		INLINE virtual physx::PxGeometry const* GetPhysicsGeometry() const override { return &mGeometry; }

	private:

		void								CalculateMeshGeometry();

		DField(Serialize)
		bool										mTightBounds;

		DField(Serialize)
		D_RESOURCE::ResourceRef<D_RENDERER::StaticMeshResource> mReferenceMesh;

		D_CORE::Uuid								mCurrentMeshUuid;
		physx::PxConvexMeshGeometry					mGeometry;
		physx::PxConvexMesh*						mMesh;

		D_GRAPHICS_BUFFERS::ReadbackBuffer			mMeshVerticesReadback;
		D_GRAPHICS_BUFFERS::ReadbackBuffer			mMeshIndicesReadback;

#if _D_EDITOR
		D_RENDERER_GEOMETRY::Mesh const*			mDebugMesh;
#endif

	};
}

File_MeshColliderComponent_GENERATED

