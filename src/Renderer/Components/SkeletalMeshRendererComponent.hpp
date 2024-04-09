#pragma once

#include "MeshRendererComponentBase.hpp"

#include "Renderer/Resources/SkeletalMeshResource.hpp"

#include "SkeletalMeshRendererComponent.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize) SkeletalMeshRendererComponent : public MeshRendererComponentBase
	{
		GENERATED_BODY();
		D_H_COMP_BODY(SkeletalMeshRendererComponent, MeshRendererComponentBase, "Rendering/Skeletal Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
		virtual void						OnGizmo() const override;
#endif

		// States
		virtual void						Update(float dt) override;
		virtual void						OnDestroy() override;
		virtual void						OnDeserialized() override;

		virtual bool						AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext) override;

		INLINE virtual UINT					GetNumberOfSubmeshes() const { return mMesh.IsValid() ? (UINT)mMesh->GetMeshData()->mDraw.size() : 0u; }

		INLINE virtual bool					CanRender() const override { return mMesh.IsValid() && MeshRendererComponentBase::CanRender(); }
		INLINE D_CONTAINERS::DVector<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>& GetSkeleton() { return mSkeleton; }
		INLINE D_RENDERER_GEOMETRY::Mesh::SkeletonJoint* GetSkeletonRoot() { return mSkeletonRoot; }


		INLINE virtual D_MATH_BOUNDS::Aabb	GetAabb() const override { return mBounds; }

		void								SetMesh(SkeletalMeshResource* mesh);
		INLINE SkeletalMeshResource*		GetMesh() const { return mMesh.Get(); }

		INLINE D_RENDERER_GEOMETRY::Mesh const* GetDeformedMeshData() const { return &mDeformedMesh; }
		virtual void						GetOverriddenMaterials(D_CONTAINERS::DVector<MaterialResource*>& out) const override;


	protected:

		DField(Serialize)
		D_RESOURCE::ResourceRef<SkeletalMeshResource>				mMesh;

	private:

		void								JointUpdateRecursion(D_MATH::Matrix4 const& parent, D_RENDERER_GEOMETRY::Mesh::SkeletonJoint& skeletonJoint);
		void								LoadMeshData();

		D_CONTAINERS::DVector<D_RENDERER::Joint>					mJoints;
		D_CONTAINERS::DVector<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint> mSkeleton;
		D_RENDERER_GEOMETRY::Mesh::SkeletonJoint*					mSkeletonRoot;
		D_MATH_BOUNDS::Aabb											mBounds;

		D_GRAPHICS_BUFFERS::UploadBuffer							mJointsBufferUpload;
		D_GRAPHICS_BUFFERS::StructuredBuffer						mJointsBufferGpu;
		D_RENDERER_GEOMETRY::Mesh									mDeformedMesh;

#if _D_EDITOR
		D_CONTAINERS::DVector<D_MATH::Vector3>						mJointLocalPoses;
#endif

	};
}

File_SkeletalMeshRendererComponent_GENERATED