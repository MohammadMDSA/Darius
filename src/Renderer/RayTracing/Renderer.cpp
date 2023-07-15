#include "Renderer/pch.hpp"
#include "Renderer.hpp"

#include "Utils/AccelerationStructure.hpp"

#ifdef _D_EDITOR
#include <imgui.h>
#endif

using namespace D_RENDERER_RT_UTILS;

namespace Darius::Renderer::RayTracing
{

	// Settings
	UINT										MaxNumBottomLevelAS;

	// Buffers
	RaytracingAccelerationStructureManager		AccelerationStructureManager;

	// Internal
	bool										_initialized;


	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		// Loading Settings
		D_H_OPTIONS_LOAD_BASIC("RayTracing.MaxNumBottomLevelAS", MaxNumBottomLevelAS);

		AccelerationStructureManager = RaytracingAccelerationStructureManager(MaxNumBottomLevelAS, D_GRAPHICS_DEVICE::gNumFrameResources);
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
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

		D_H_OPTION_DRAW_INT_SLIDER("Max Number of Buttom Level AS", "RayTracing.MaxNumBottomLevelAS", MaxNumBottomLevelAS, 10, 10000000);

		D_H_OPTION_DRAW_END();
	}
#endif

}