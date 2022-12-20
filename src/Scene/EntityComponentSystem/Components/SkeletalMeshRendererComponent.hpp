#pragma once

#include "ComponentBase.hpp"

#include <Core/Ref.hpp>
#include <Renderer/Resources/SkeletalMeshResource.hpp>
#include <Renderer/Resources/MaterialResource.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

using namespace D_CORE;
using namespace D_RESOURCE;

namespace Darius::Scene::ECS::Components
{
	class SkeletalMeshRendererComponent : public ComponentBase
	{
		D_H_COMP_BODY(SkeletalMeshRendererComponent, ComponentBase, "Rendering/Skeletal Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		// Serialization
		virtual void						Serialize(D_SERIALIZATION::Json& j) const override;
		virtual void						Deserialize(D_SERIALIZATION::Json const& j) override;

		// States
		virtual void						Start() override;
		virtual void						Update(float dt) override;
		virtual void						OnDestroy() override;

		void								SetMesh(ResourceHandle handle);
		void								SetMaterial(ResourceHandle handle);
		INLINE bool							HasAnimation() const { return true; }

		RenderItem							GetRenderItem();
		INLINE DVector<Mesh::SkeletonJoint>& GetSkeleton() { return mSkeleton; }
		INLINE Mesh::SkeletonJoint* GetSkeletonRoot() { return mSkeletonRoot; }


		INLINE bool							CanRender() { return IsActive() && mMeshResource.IsValid(); }
		INLINE const BoundingSphere& GetBounds() const { return mBounds; }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsAddress() { return mMeshConstantsGPU.GetGpuVirtualAddress(); }


		D_CH_RW_FIELD_ACC(Ref<SkeletalMeshResource>, MeshResource, protected);
		D_CH_RW_FIELD_ACC(Ref<MaterialResource>, MaterialResource, protected);

	private:

		void								_SetMesh(ResourceHandle handle);
		void								_SetMaterial(ResourceHandle handle);
		void								CreateGPUBuffers();
		void								JointUpdateRecursion(Matrix4 const& parent, Mesh::SkeletonJoint& skeletonJoint);
		INLINE uint16_t						GetPsoIndex()
		{
			auto materialPsoFlags = mMaterialResource->GetPsoFlags();
			
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

		DVector<Joint>						mJoints;
		DVector<Mesh::SkeletonJoint>		mSkeleton;
		Mesh::SkeletonJoint*				mSkeletonRoot;
		BoundingSphere						mBounds;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer	mMeshConstantsCPU[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
		ByteAddressBuffer					mMeshConstantsGPU;

		uint16_t							mComponentPsoFlags;
		uint16_t							mCachedMaterialPsoFlags;
		uint16_t							mPsoIndex;
		bool								mPsoIndexDirty;

	};
}