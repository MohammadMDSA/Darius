#pragma once

#include <Core/Ref.hpp>
#include <Renderer/Resources/SkeletalMeshResource.hpp>
#include <Renderer/Resources/MaterialResource.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "SkeletalMeshRendererComponent.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DClass(Serialize) SkeletalMeshRendererComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(SkeletalMeshRendererComponent, D_ECS_COMP::ComponentBase, "Rendering/Skeletal Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		// States
		virtual void						Awake() override;
		virtual void						Update(float dt) override;
		virtual void						OnDestroy() override;

		INLINE bool							HasAnimation() const { return true; }

		D_RENDERER_FRAME_RESOURCE::RenderItem GetRenderItem();
		INLINE D_CONTAINERS::DVector<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint>& GetSkeleton() { return mSkeleton; }
		INLINE D_RENDERER_GEOMETRY::Mesh::SkeletonJoint* GetSkeletonRoot() { return mSkeletonRoot; }


		INLINE bool							CanRender() { return IsActive() && mMesh.IsValid() && !mMaterial->IsDirtyGPU(); }
		INLINE const D_MATH_BOUNDS::BoundingSphere& GetBounds() const { return mBounds; }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsAddress() { return mMeshConstantsGPU.GetGpuVirtualAddress(); }



	protected:

		DField(Resource[false], Serialize)
		D_CORE::Ref<MaterialResource>		mMaterial;

		DField(Resource[false], Serialize)
		D_CORE::Ref<SkeletalMeshResource>	mMesh;

	private:

		void								_SetMesh(D_RESOURCE::ResourceHandle handle);
		void								_SetMaterial(D_RESOURCE::ResourceHandle handle);
		void								CreateGPUBuffers();
		void								JointUpdateRecursion(D_MATH::Matrix4 const& parent, D_RENDERER_GEOMETRY::Mesh::SkeletonJoint& skeletonJoint);

		INLINE uint16_t						GetPsoIndex()
		{
			auto materialPsoFlags = mMaterial->GetPsoFlags();
			
			// Whether resource has changed
			if (mCachedMaterialPsoFlags != materialPsoFlags)
			{
				mCachedMaterialPsoFlags = materialPsoFlags;
				mPsoIndexDirty = true;
			}

			// Whether pso index is not compatible with current pso flags
			if (mPsoIndexDirty)
			{
				mPsoIndex = D_RENDERER::GetPso(materialPsoFlags | mComponentPsoFlags);
				mPsoIndexDirty = false;
			}
			return mPsoIndex;
		}

		DField(Get[inline], Set[inline], Serialize)
		bool														mCastsShadow;

		D_CONTAINERS::DVector<D_RENDERER_FRAME_RESOURCE::Joint>		mJoints;
		D_CONTAINERS::DVector<D_RENDERER_GEOMETRY::Mesh::SkeletonJoint> mSkeleton;
		D_RENDERER_GEOMETRY::Mesh::SkeletonJoint*					mSkeletonRoot;
		D_MATH_BOUNDS::BoundingSphere								mBounds;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer							mMeshConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
		D_GRAPHICS_BUFFERS::ByteAddressBuffer						mMeshConstantsGPU;

		uint16_t													mComponentPsoFlags;
		uint16_t													mCachedMaterialPsoFlags;
		uint16_t													mPsoIndex;
		bool														mPsoIndexDirty;

	public:
		Darius_Graphics_SkeletalMeshRendererComponent_GENERATED

	};
}

File_SkeletalMeshRendererComponent_GENERATED