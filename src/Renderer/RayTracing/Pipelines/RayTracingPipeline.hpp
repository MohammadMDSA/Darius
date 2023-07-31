#pragma once

#include "Renderer/RayTracing/RayTracingCommandContext.hpp"

#include <Graphics/GraphicsUtils/StateObject.hpp>

#include <Core/Serialization/Json.hpp>

#ifndef D_RENDERER_RT_PIPELINE
#define D_RENDERER_RT_PIPELINE Darius::Renderer::RayTracing::Pipeline
#endif

namespace Darius::Renderer::RayTracing::Pipeline
{
	interface IRayTracingPipeline
	{
		virtual void Initialize(D_SERIALIZATION::Json const& settings) = 0;
		virtual void Shutdown() = 0;
		virtual D_GRAPHICS_UTILS::RayTracingStateObject const* GetStateObject() const = 0;
	};
}
