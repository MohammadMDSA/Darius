#pragma once

#include "ComponentBase.hpp"

#include <Core/Ref.hpp>
#include <ResourceManager/ResourceTypes/SkeletalMeshResource.hpp>
#include <ResourceManager/ResourceTypes/MaterialResource.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

using namespace D_CORE;
using namespace D_RESOURCE;

namespace Darius::Scene::ECS::Components
{
	class SkeletalMeshRendererComponent : public ComponentBase
	{
		D_H_COMP_BODY(SkeletalMeshRendererComponent, ComponentBase, "Rendering/Skeletal Mesh Renderer", true, false);

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
		INLINE Mesh::SkeletonJoint*			GetSkeletonRoot() { return mSkeletonRoot; }


		INLINE bool							CanRender() { return IsActive() && mMeshResource.IsValid(); }
		INLINE const BoundingSphere&		GetBounds() const { return mBounds; }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsAddress() { return mMeshConstantsGPU.GetGpuVirtualAddress(); }


		D_CH_RW_FIELD_ACC(Ref<SkeletalMeshResource>, MeshResource, protected);
		D_CH_RW_FIELD_ACC(Ref<MaterialResource>, MaterialResource, protected);

	private:

		void								_SetMesh(ResourceHandle handle);
		void								_SetMaterial(ResourceHandle handle);
		void								CreateGPUBuffers();
		void								JointUpdateRecursion(Matrix4 const& parent, Mesh::SkeletonJoint& skeletonJoint);

		D_CORE::Signal<void()>				mChangeSignal;
		uint16_t							mPsoFlags;
		DVector<Joint>						mJoints;
		DVector<Mesh::SkeletonJoint>		mSkeleton;
		Mesh::SkeletonJoint*				mSkeletonRoot;
		BoundingSphere						mBounds;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer	mMeshConstantsCPU[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
		ByteAddressBuffer					mMeshConstantsGPU;

	};
}