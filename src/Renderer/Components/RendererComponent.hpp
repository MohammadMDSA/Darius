#pragma once

#include "IRenderable.hpp"

#include <Math/Bounds/DynamicBVH.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "RendererComponent.generated.hpp"

#ifndef	D_RENDERER
#define D_RENDERER Darius::Renderer
#endif

namespace Darius::Renderer
{
	class DClass(Serialize) RendererComponent : public D_ECS_COMP::ComponentBase, public IRenderable
	{
		GENERATED_BODY();
		D_H_COMP_BODY(RendererComponent, D_ECS_COMP::ComponentBase, "Rendering/Renderable", false);
		
	public:

		virtual void										Start() override;
		virtual void										Awake() override;
		virtual void										Update(float dt) override;
		virtual void										OnActivate() override;
		virtual void										OnDeactivate() override;
		virtual void										OnPreDestroy() override;
		virtual void										OnDestroy() override;

		INLINE virtual bool									CanRender() const override { return IsActive(); }
		INLINE virtual D_MATH_BOUNDS::Aabb					GetAabb() const override { D_ASSERT(false); return D_MATH_BOUNDS::Aabb(); }
		INLINE virtual D3D12_GPU_VIRTUAL_ADDRESS			GetConstantsAddress() const override { return D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
		INLINE virtual bool									AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext) override { return false; }
#if _D_EDITOR
		INLINE virtual D_RENDERER::RenderItem				GetPickerRenderItem() const override { D_ASSERT_NOENTRY(); return {}; }
#endif
		INLINE virtual bool									IsCastingShadow() const override { return mCastsShadow; }

		void												SetCastsShadow(bool value);

		INLINE bool											IsStencilWriteEnable() const { return mStencilWriteEnable; }
		INLINE UINT8										GetStencilValue() const { return mStencilValue; }
		INLINE bool											IsCustomDepthEnable() const { return mCustomDepthEnable; }

		void												SetStencilWriteEnable(bool value);
		void												SetStencilValue(UINT8 value);
		void												SetCustomDepthEnable(bool value);

#ifdef _D_EDITOR
		virtual bool										DrawDetails(float[]) override;

		INLINE D3D12_GPU_VIRTUAL_ADDRESS							GetPickerDrawPsConstant() const { return mPickerDrawPsConstant.GetGpuVirtualAddress(); }
#endif // _D_EDITOR


	private:

		DField(Serialize)
		bool												mCastsShadow;

		DField(Serialize)
		bool												mStencilWriteEnable;

		DField(Serialize)
		bool												mCustomDepthEnable;

		DField(Serialize)
		UINT8												mStencilValue;

#if _D_EDITOR
		D_GRAPHICS_BUFFERS::ByteAddressBuffer				mPickerDrawPsConstant;
#endif

		D_MATH_BOUNDS::DynamicBVH<D_ECS::UntypedCompRef>::ID mBvhNodeId;
	};
}
