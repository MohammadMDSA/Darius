#include "Renderer/pch.hpp"
#include "Renderer.hpp"

#include "Pipelines/SimpleRayTracingRenderer.hpp"
#include "Utils/AccelerationStructure.hpp"

#ifdef _D_EDITOR
#include <imgui.h>
#endif

using namespace D_RENDERER_RT_UTILS;

namespace Darius::Renderer::RayTracing
{

	// Settings
	UINT													MaxNumBottomLevelAS;

	// Buffers
	std::unique_ptr<RaytracingAccelerationStructureManager>	AccelerationStructureManager;

	// Renderers
	std::unique_ptr<Pipeline::SimpleRayTracingPipeline>		SimpleRayTracingRenderer;

	// Internal
	bool													_initialized;

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		// Loading Settings
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("RayTracing.MaxNumBottomLevelAS", MaxNumBottomLevelAS, 100000);

		AccelerationStructureManager = std::make_unique<RaytracingAccelerationStructureManager>(MaxNumBottomLevelAS, (UINT)D_GRAPHICS_DEVICE::gNumFrameResources);
		AccelerationStructureManager->InitializeTopLevelAS(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE, false, false, L"Top-Level Acceleration Structure");

		SimpleRayTracingRenderer = std::make_unique<Pipeline::SimpleRayTracingPipeline>();
		SimpleRayTracingRenderer->Initialize(settings);
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		AccelerationStructureManager.reset();
		SimpleRayTracingRenderer.reset();
	}

	void Update()
	{

	}

	void Render(std::wstring const& jobId, RayTracingCommandContext& context)
	{

	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		D_H_OPTION_DRAW_INT_SLIDER("Max Number of Buttom Level AS", "RayTracing.MaxNumBottomLevelAS", MaxNumBottomLevelAS, 10, 1000000);

		D_H_OPTION_DRAW_END();
	}
#endif

}