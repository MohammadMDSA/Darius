#pragma once

#include <Core/Ref.hpp>
#include <Renderer/Resources/StaticMeshResource.hpp>
#include <Renderer/Resources/MaterialResource.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class MeshRendererComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(MeshRendererComponent, D_ECS_COMP::ComponentBase, "Rendering/Mesh Renderer", true);

	public:

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif

		// Serialization
		virtual void						Serialize(D_SERIALIZATION::Json& j) const override;
		virtual void						Deserialize(D_SERIALIZATION::Json const& j) override;

		// States
		virtual void						Awake() override;
		virtual void						Update(float dt) override;
		virtual void						OnDestroy() override;

		void								SetMesh(D_RESOURCE::ResourceHandle handle);
		void								SetMaterial(D_RESOURCE::ResourceHandle handle);

		D_RENDERER_FRAME_RESOURCE::RenderItem GetRenderItem();


		INLINE bool							CanRender() { return IsActive() && mMeshResource.IsValid(); }
		INLINE const D_MATH_BOUNDS::BoundingSphere& GetBounds() const { return mMeshResource.Get()->GetMeshData()->mBoundSp; }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsAddress() { return mMeshConstantsGPU.GetGpuVirtualAddress(); }

		D_CH_RW_FIELD(bool, CastsShadow);

	private:

		void								_SetMesh(D_RESOURCE::ResourceHandle handle);
		void								_SetMaterial(D_RESOURCE::ResourceHandle handle);

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


		D_CORE::Ref<StaticMeshResource>		mMeshResource;
		D_CORE::Ref<MaterialResource>		mMaterialResource;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer	mMeshConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
		D_GRAPHICS_BUFFERS::ByteAddressBuffer mMeshConstantsGPU;

		uint16_t							mComponentPsoFlags;
		uint16_t							mCachedMaterialPsoFlags;
		uint16_t							mPsoIndex;
		bool								mPsoIndexDirty;

	};
}