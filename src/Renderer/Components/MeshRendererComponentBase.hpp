#pragma once

#include "IRenderable.hpp"
#include "Renderer/Resources/MaterialResource.hpp"
#include "RendererComponent.hpp"

#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include "MeshRendererComponentBase.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize) MeshRendererComponentBase : public RendererComponent
	{
		GENERATED_BODY();
		D_H_COMP_BODY(MeshRendererComponentBase, RendererComponent, "Rendering/Base Mesh Renderer", false);

	public:

		// States
		virtual void							Awake() override;
		virtual void							OnDestroy() override;

		virtual void							OnDeserialized() override;

#ifdef _D_EDITOR
		virtual bool							DrawDetails(float params[]) override;
#endif

		INLINE virtual bool						AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext) override { return false; }
		
		INLINE virtual UINT						GetNumberOfSubmeshes() const { return 0; }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS		GetConstantsAddress() const override { return mMeshConstantsGPU.GetGpuVirtualAddress(); }
		INLINE virtual bool						CanRender() const override { return Super::CanRender(); }
		INLINE virtual bool						IsDirty() const override { return D_ECS_COMP::ComponentBase::IsDirty() || GetTransform()->IsDirty(); }

		void									SetMaterial(UINT index, MaterialResource* material);
		D_CONTAINERS::DVector<D_RESOURCE::ResourceRef<MaterialResource>> const& GetMaterials() const { return mMaterials; }
		D_RESOURCE::ResourceRef<MaterialResource> GetMaterial(UINT index) const { D_ASSERT(index < (UINT)mMaterials.size()); return mMaterials[index]; }
		virtual void							GetOverriddenMaterials(D_CONTAINERS::DVector<MaterialResource*>& out) const {}

	protected:

		virtual UINT							GetPsoIndex(UINT materialIndex, MaterialResource* material);
		void									OnMeshChanged();
		
		DField(Serialize)
		D_CONTAINERS::DVector<D_RESOURCE::ResourceRef<MaterialResource>> mMaterials;

		DField(Get[inline], Set[inline, dirty], Serialize)
		float									mLoD;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer		mMeshConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	mMeshConstantsGPU;

		uint16_t								mComponentPsoFlags;

		D_CONTAINERS::DVector<MaterialPsoData>	mMaterialPsoData;
		
	};
}

File_MeshRendererComponentBase_GENERATED
