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
	class DClass(Serialize[bTightBounds, bDirectMesh, bConvex]) MeshColliderComponent : public ColliderComponent
	{
		GENERATED_BODY();
		D_H_COMP_BODY(MeshColliderComponent, ColliderComponent, "Physics/Mesh Collider", true);

	public:

		void								Awake() override;
		void								OnDestroy() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
		virtual void						OnGizmo() const override;
		virtual void						OnPostComponentAddInEditor() override;
#endif // _D_EDITOR

		virtual bool						CalculateGeometry(_OUT_ physx::PxGeometry & geom) const override;
		INLINE virtual bool					UpdateGeometry() override { return CalculateGeometry(mGeometry); }

		INLINE bool							IsTightBounds() const { return mTightBounds; }
		void								SetTightBounds(bool tight);

		// Direct
		INLINE bool							IsDirectMesh() const { return mDirectMesh; }
		void								SetDirectMesh(bool direct);

		// Convex
		INLINE bool							IsConvex() const { return mConvex; }
		void								SetConvex(bool convex);

		D_RENDERER::StaticMeshResource*		GetReferenceMesh() const { return mReferenceMesh.Get(); }
		void								SetReferenceMesh(D_RENDERER::StaticMeshResource* staticMesh);

	protected:
		INLINE virtual physx::PxGeometry const* GetPhysicsGeometry() const override { return &mGeometry; }

	private:

		// Only uses the first index buffer.
		bool								CalculateMeshGeometry();
		// Only uses the first index buffer.
		void								CalculateConvexMeshGeometry();
		// Only uses the first index buffer.
		void								CalculateTriangleMeshGeometry();

		UINT										mTightBounds : 1;
		UINT										mDirectMesh : 1;
		UINT										mConvex : 1;

		DField(Serialize)
		D_RESOURCE::ResourceRef<D_RENDERER::StaticMeshResource> mReferenceMesh;

		D_CORE::Uuid								mCurrentMeshUuid;
		physx::PxConvexMeshGeometry					mGeometry;
		MeshDataHandle								mMesh;

		D_GRAPHICS_BUFFERS::ReadbackBuffer			mMeshVerticesReadback;
		D_GRAPHICS_BUFFERS::ReadbackBuffer			mMeshIndicesReadback;

	};
}

File_MeshColliderComponent_GENERATED

