#pragma once

#include "IRenderable.hpp"

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
		INLINE virtual bool									CanRender() const override { return IsActive(); }
		INLINE virtual D_MATH_BOUNDS::BoundingSphere const& GetBounds() override { return D_MATH_BOUNDS::BoundingSphere(); }
		INLINE virtual D3D12_GPU_VIRTUAL_ADDRESS			GetConstantsAddress() const override { return D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
		INLINE virtual bool									AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction) override { return false; }
		INLINE virtual bool									IsCastingShadow() const override { return mCastsShadow; }

		void												SetCastsShadow(bool value);

#ifdef _D_EDITOR
		virtual bool										DrawDetails(float[]) override;
#endif

	private:

		DField(Serialize)
		bool									mCastsShadow;
	};
}
