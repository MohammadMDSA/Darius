#pragma once

#include <Core/Ref.hpp>
#include <Renderer/Resources/StaticMeshResource.hpp>
#include <Renderer/Resources/MaterialResource.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "MeshRendererComponent.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DClass(Serialize) MeshRendererComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(MeshRendererComponent, D_ECS_COMP::ComponentBase, "Rendering/Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		// States
		virtual void						Awake() override;
		virtual void						Update(float dt) override;
		virtual void						OnDestroy() override;

		D_RENDERER_FRAME_RESOURCE::RenderItem GetRenderItem();


		INLINE bool							CanRender() { return IsActive() && mMesh.IsValid(); }
		INLINE const D_MATH_BOUNDS::BoundingSphere& GetBounds() const { return mMesh.Get()->GetMeshData()->mBoundSp; }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsAddress() { return mMeshConstantsGPU.GetGpuVirtualAddress(); }

	private:

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

		DField(Get[inline], Set[inline])
		bool								mCastsShadow;

		DField(Resource, Serialize)
		D_CORE::Ref<StaticMeshResource>		mMesh;

		DField(Resource, Serialize)
		D_CORE::Ref<MaterialResource>		mMaterial;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer	mMeshConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
		D_GRAPHICS_BUFFERS::ByteAddressBuffer mMeshConstantsGPU;

		uint16_t							mComponentPsoFlags;
		uint16_t							mCachedMaterialPsoFlags;
		uint16_t							mPsoIndex;
		bool								mPsoIndexDirty;

	public:
		Darius_Graphics_MeshRendererComponent_GENERATED

	};
}

File_MeshRendererComponent_GENERATED