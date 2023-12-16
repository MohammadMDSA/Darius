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

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float params[]) override;
#endif // _D_EDITOR


		virtual bool						AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext) override;

		INLINE virtual D3D12_GPU_VIRTUAL_ADDRESS GetConstantsAddress() const override { return mMeshConstantsGPU.GetGpuVirtualAddress(); }
		INLINE virtual D_MATH_BOUNDS::BoundingSphere const& GetBounds() override
		{
			if (mBoundDirty)
			{
				UpdateBound();
				mBoundDirty = false;
			}
			return mBoundingSphere;
		}

		void								SetWidth(float const& value);
		void								SetHeight(float const& value);

		void								SetMaterial(MaterialResource* material);

		INLINE virtual bool					IsDirty() const override { return D_ECS_COMP::ComponentBase::IsDirty() || GetTransform()->IsDirty(); }

	protected:
		void								UpdatePsoIndex();

		DField(Serialize)
		D_RESOURCE::ResourceRef<MaterialResource> mMaterial;

	private:
		
		INLINE void							UpdateBound() { mBoundingSphere = D_MATH_BOUNDS::BoundingSphere(D_MATH::Vector3::Zero, D_MATH::Length(D_MATH::Vector3::Up * mWidth + D_MATH::Vector3::Forward * mHeight)); }

		DField(Serialize, Get[inline])
		float								mWidth;

		DField(Serialize, Get[inline])
		float								mHeight;

		bool								mBoundDirty;
		D_MATH_BOUNDS::BoundingSphere		mBoundingSphere;


		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer		mMeshConstantsCPU;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	mMeshConstantsGPU;


		MaterialPsoData						mMaterialPsoData;
	
	};
}

File_BillboardRendererComponent_GENERATED
