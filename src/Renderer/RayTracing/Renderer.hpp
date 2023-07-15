#pragma once

#include "RayTracingCommandContext.hpp"

#include <Core/Serialization/Json.hpp>

#define D_RENDERER_RT Darius::Renderer::RayTracing

namespace Darius::Renderer::RayTracing
{
	void Initialize(D_SERIALIZATION::Json const& settings);
	void Shutdown();
	void Update();
	void Render(std::wstring const& jobId, RayTracingCommandContext& context);


#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif

}