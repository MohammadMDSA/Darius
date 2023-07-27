#pragma once

#include "Utils/AccelerationStructure.hpp"

#ifndef D_RENDERER_RT
#define D_RENDERER_RT Darius::Renderer::RayTracing
#endif // !D_RENDERER_RT_UTILS

namespace Darius::Renderer::RayTracing
{
	class RayTracingScene
	{
	public:
		RayTracingScene(UINT maxNumBLASes);

	private:
		std::unique_ptr<D_RENDERER_RT_UTILS::RaytracingAccelerationStructureManager>	mAccelerationStructureManager;

	};
}