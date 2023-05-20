#pragma once

#include "IRenderable.hpp"

#include "Renderer/Resources/MaterialResource.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "BillboardRendererComponent.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif // !D_GRAPHICS

namespace Darius::Graphics
{
	class DClass(Serialize) BillboardRendererComponent : public D_ECS_COMP::ComponentBase, public IRenderable
	{

		D_H_COMP_BODY(BillboardRendererComponent, D_ECS_COMP::ComponentBase, "Rendering/Billboard Renderer", true);

	public:

		virtual void						Update(float dt) override;
		virtual void						Awake() override;

#ifdef _D_EDITOR
		virtual bool						DrawDetails(float[]) override;
#endif // _D_EDITOR


		virtual bool						AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction);

		INLINE virtual bool					CanRender() const { return IsActive(); }
		INLINE virtual D3D12_GPU_VIRTUAL_ADDRESS GetConstantsAddress() const override { return mMeshConstantsGPU.GetGpuVirtualAddress(); }
		INLINE virtual D_MATH_BOUNDS::BoundingSphere const& GetBounds()
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

	protected:
		void								UpdatePso();

		DField(Serialize, Resource)
		D_RESOURCE::ResourceRef<MaterialResource> mMaterial;

	private:
		
		INLINE void							UpdateBound() { mBoundingSphere = D_MATH_BOUNDS::BoundingSphere(D_MATH::Vector3::Zero, D_MATH::Length(D_MATH::Vector3::Up * mWidth + D_MATH::Vector3::Forward * mHeight)); }

		DField(Serialize, Get[inline])
		float								mWidth;

		DField(Serialize, Get[inline])
		float								mHeight;

		DField(Get[inline], Set[inline])
		bool								mCastsShadow;

		bool								mBoundDirty;
		D_MATH_BOUNDS::BoundingSphere		mBoundingSphere;


		// Gpu buffers
		D_GRAPHICS_BUFFERS::UploadBuffer		mMeshConstantsCPU[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
		D_GRAPHICS_BUFFERS::ByteAddressBuffer	mMeshConstantsGPU;


		static UINT							RenderPsoIndex;
		static UINT							DepthPsoIndex;
	

	public:
		Darius_Graphics_BillboardRendererComponent_GENERATED
	};
}

File_BillboardRendererComponent_GENERATED
