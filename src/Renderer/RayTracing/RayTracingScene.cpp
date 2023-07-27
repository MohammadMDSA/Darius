#include "Renderer/pch.hpp"
#include "RayTracingScene.hpp"

using namespace D_RENDERER_RT_UTILS;

namespace Darius::Renderer::RayTracing
{
	RayTracingScene::RayTracingScene(UINT maxNumBLASes)
	{
		mAccelerationStructureManager = std::make_unique<RaytracingAccelerationStructureManager>(maxNumBLASes, (UINT)D_GRAPHICS_DEVICE::gNumFrameResources);

		mAccelerationStructureManager->InitializeTopLevelAS(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE, false, false, L"Top Level Acceleration Structure");
	}
}