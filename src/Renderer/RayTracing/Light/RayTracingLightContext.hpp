#pragma once

#include "Renderer/Light/LightContext.hpp"

#ifndef D_RENDERER_RT_LIGHT
#define D_RENDERER_RT_LIGHT Darius::Renderer::RayTracing::Light
#endif // !D_RENDERER_RT_LIGHT

namespace Darius::Renderer::RayTracing::Light
{
	class RayTracingLightContext : public D_RENDERER_LIGHT::LightContext
	{
	public:
		virtual void						Update(D_MATH_CAMERA::Camera const& viewerCamera) override;

	};
}
