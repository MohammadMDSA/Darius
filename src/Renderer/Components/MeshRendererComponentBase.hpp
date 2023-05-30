#pragma once

#include "IRenderable.hpp"
#include "Renderer/Resources/MaterialResource.hpp"

#include <Core/Ref.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "MeshRendererComponentBase.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DClass(Serialize) MeshRendererComponentBase : public D_ECS_COMP::ComponentBase, public IRenderable
	{
		D_H_COMP_BODY(MeshRendererComponentBase, D_ECS_COMP::ComponentBase, "Rendering/Base Mesh Renderer", false);

	public:

		// States
		virtual void							Awake() override;
		virtual void							OnDestroy() override;

		virtual void							OnDeserialized() override;

#ifdef _D_EDITOR
		virtual bool							DrawDetails(float params[]) override;
#endif

		INLINE virtual bool						AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction) override { return false; }
		
		INLINE virtual UINT						GetNumberOfSubmeshes() const { return 0; }

		virtual bool							CanRender() const override { return IsActive(); }

		INLINE virtual D_MATH_BOUNDS::BoundingSphere const& GetBounds() override { return (D_MATH_BOUNDS::BoundingSphere&)*this; }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS		GetConstantsAddress() const override { return mMeshConstantsGPU.GetGpuVirtualAddress(); }

	protected:

		void									_SetMaterial(UINT index, D_RESOURCE::ResourceHandle handle);
		UINT									GetPsoIndex(UINT materialIndex);
		void									OnMeshChanged();
		
		DField(Serialize)
		D_CONTAINERS::DVector<D_RESOURCE::ResourceRef<MaterialResource>> mMaterials;

		DField(Get[inline], Set[inline])
		bool									mCastsShadow;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer		mMeshConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	mMeshConstantsGPU;

		uint16_t								mComponentPsoFlags;

		D_CONTAINERS::DVector<MaterialPsoData>	mMaterialPsoData;
		

	public:
		Darius_Graphics_MeshRendererComponentBase_GENERATED
	};
}

File_MeshRendererComponentBase_GENERATED
