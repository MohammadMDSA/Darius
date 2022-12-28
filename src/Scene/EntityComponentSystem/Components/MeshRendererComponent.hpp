#pragma once

#include "ComponentBase.hpp"

#include <Core/Ref.hpp>
#include <Renderer/Resources/StaticMeshResource.hpp>
#include <Renderer/Resources/MaterialResource.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

using namespace D_CORE;
using namespace D_RESOURCE;

namespace Darius::Scene::ECS::Components
{
	class MeshRendererComponent : public ComponentBase
	{
		D_H_COMP_BODY(MeshRendererComponent, ComponentBase, "Rendering/Mesh Renderer", true);

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

		RenderItem							GetRenderItem();


		INLINE bool							CanRender() { return IsActive() && mMeshResource.IsValid(); }
		INLINE const BoundingSphere&		GetBounds() const { return mMeshResource.Get()->GetMeshData()->mBoundSp; }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsAddress() { return mMeshConstantsGPU.GetGpuVirtualAddress(); }

		D_CH_RW_FIELD(bool,					CastsShadow);

	private:

		void								_SetMesh(ResourceHandle handle);
		void								_SetMaterial(ResourceHandle handle);

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


		Ref<StaticMeshResource>				mMeshResource;
		Ref<MaterialResource>				mMaterialResource;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer	mMeshConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
		ByteAddressBuffer					mMeshConstantsGPU;

		uint16_t							mComponentPsoFlags;
		uint16_t							mCachedMaterialPsoFlags;
		uint16_t							mPsoIndex;
		bool								mPsoIndexDirty;

	};
}