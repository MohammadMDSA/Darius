#pragma once

#include "MeshRendererComponentBase.hpp"

#include <Core/Ref.hpp>
#include <Graphics/Resources/SkeletalMeshResource.hpp>

#include "SkeletalMeshRendererComponent.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DClass(Serialize) SkeletalMeshRendererComponent : public MeshRendererComponentBase
	{
		D_H_COMP_BODY(SkeletalMeshRendererComponent, MeshRendererComponentBase, "Rendering/Skeletal Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		// States
		virtual void						Update(float dt) override;
		virtual void						OnDeserialized() override;

		virtual bool						AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction) override;

		INLINE virtual UINT					GetNumberOfSubmeshes() const { return mMesh.IsValid() ? (UINT)mMesh->GetMeshData()->mDraw.size() : 0u; }

		INLINE virtual bool					CanRender() const override { return MeshRendererComponentBase::CanRender() && mMesh.IsValid(); }
		INLINE D_CONTAINERS::DVector<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>& GetSkeleton() { return mSkeleton; }
		INLINE D_RENDERER_GEOMETRY::Mesh::SkeletonJoint* GetSkeletonRoot() { return mSkeletonRoot; }


		INLINE virtual D_MATH_BOUNDS::BoundingSphere const& GetBounds() override { return mBounds; }


	protected:

		DField(Resource[false], Serialize)
		D_RESOURCE::ResourceRef<SkeletalMeshResource>				mMesh;

	private:

		void								_SetMesh(D_RESOURCE::ResourceHandle handle);
		void								JointUpdateRecursion(D_MATH::Matrix4 const& parent, D_RENDERER_GEOMETRY::Mesh::SkeletonJoint& skeletonJoint);
		void								LoadMeshData();

		D_CONTAINERS::DVector<D_RENDERER_FRAME_RESOURCE::Joint>		mJoints;
		D_CONTAINERS::DVector<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint> mSkeleton;
		D_RENDERER_GEOMETRY::Mesh::SkeletonJoint*					mSkeletonRoot;
		D_MATH_BOUNDS::BoundingSphere								mBounds;

		
	public:
		Darius_Graphics_SkeletalMeshRendererComponent_GENERATED

	};
}

File_SkeletalMeshRendererComponent_GENERATED