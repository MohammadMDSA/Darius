#pragma once

#include "Renderer/FrameGraph/RenderPass.hpp"

#ifndef D_RENDERER_RAST
#define D_RENDERER_RAST Darius::Renderer::Rasterization
#endif

namespace Darius::Renderer::Rasterization
{
	class RasterizationSkyboxPass : D_RENDERER::RenderPass
	{
		D_H_RENDER_PASS_FACTORY(RasterizationSkyboxPass);

	public:

	};
}
