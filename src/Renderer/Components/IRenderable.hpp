#pragma once

#include "Renderer/FrameResource.hpp"

#include <Math/Bounds/BoundingSphere.hpp>

#include <rttr/rttr_enable.h>

#include <functional>

#include "IRenderable.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DClass(Serialize) IRenderable
	{
	public:
		virtual bool						CanRender() const = 0;
		virtual D_MATH_BOUNDS::BoundingSphere const& GetBounds() = 0;
		virtual D3D12_GPU_VIRTUAL_ADDRESS	GetConstantsAddress() const = 0;
		virtual bool						AddRenderItems(std::function<void(D_RENDERER_FRAME_RESOURCE::RenderItem const&)> appendFunction) = 0;
		virtual bool						IsCastsShadow() const = 0;

		Darius_Graphics_IRenderable_GENERATED
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
