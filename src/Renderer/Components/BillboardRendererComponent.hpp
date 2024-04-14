#pragma once

#include "IRenderable.hpp"

#include "Renderer/Resources/MaterialResource.hpp"
#include "RendererComponent.hpp"

#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include "BillboardRendererComponent.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize) BillboardRendererComponent : public RendererComponent
	{

		GENERATED_BODY();
		D_H_COMP_BODY(BillboardRendererComponent, RendererComponent, "Rendering/Billboard Renderer", true);

	public:

		virtual void						Update(float dt) override;
		virtual void						Awake() override;
		virtual void						OnDestroy() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif // _D_EDITOR


		virtual bool						AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext) override;

		INLINE virtual D3D12_GPU_VIRTUAL_ADDRESS GetConstantsAddress() const override { return mMeshConstantsGPU.GetGpuVirtualAddress(); }
		virtual D_MATH_BOUNDS::Aabb			GetAabb() const override;
		void								SetWidth(float const& value);
		void								SetHeight(float const& value);

		void								SetMaterial(MaterialResource* material);

		INLINE virtual bool					IsDirty() const override { return D_ECS_COMP::ComponentBase::IsDirty() || GetTransform()->IsDirty(); }

	protected:
		void								UpdatePsoIndex();

		DField(Serialize)
		D_RESOURCE::ResourceRef<MaterialResource> mMaterial;

	private:
		
		DField(Serialize, Get[inline])
		float								mWidth;

		DField(Serialize, Get[inline])
		float								mHeight;

		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer	mMeshConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	mMeshConstantsGPU;


		MaterialPsoData						mMaterialPsoData;
	
	};
}

File_BillboardRendererComponent_GENERATED
