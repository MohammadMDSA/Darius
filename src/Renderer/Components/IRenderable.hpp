#pragma once

#include "Renderer/RendererCommon.hpp"

#include <Math/Bounds/BoundingBox.hpp>

#include <rttr/rttr_enable.h>

#include <functional>

#include "IRenderable.generated.hpp"

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class DClass(Serialize) IRenderable
	{
		GENERATED_BODY();

	public:
		virtual bool						CanRender() const = 0;
		virtual D_MATH_BOUNDS::Aabb			GetAabb() const = 0;
		virtual D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsAddress() const = 0;
		virtual bool						AddRenderItems(std::function<void(D_RENDERER::RenderItem const&)> appendFunction, RenderItemContext const& riContext) = 0;
		virtual bool						IsCastingShadow() const = 0;
#if _D_EDITOR
		virtual D_RENDERER::RenderItem		GetPickerRenderItem() const = 0;
#endif
	};

	struct MaterialPsoData
	{
		uint16_t							CachedMaterialPsoFlags = 0;
		UINT								PsoIndex = 0;
		UINT								DepthPsoIndex = 0;
		bool								PsoIndexDirty = true;
	};
}

File_IRenderable_GENERATED
