#pragma once

#include "RayTracingPipeline.hpp"

#include <Graphics/GraphicsUtils/StateObject.hpp>

#ifndef D_RENDERER_RT_PIPELINE
#define D_RENDERER_RT_PIPELINE Darius::Renderer::RayTracing::Pipeline
#endif

namespace Darius::Renderer::RayTracing::Pipeline
{
	class SimpleRayTracingPipeline : public IRayTracingPipeline
	{
	public:
		SimpleRayTracingPipeline();

		virtual void	Initialize(D_SERIALIZATION::Json const& settings) override;
		virtual void	Shutdown() override;
		virtual void	Update() override;
		virtual void	Render(std::wstring const& jobId, RayTracingCommandContext& context) override;

	private:

		void			CreateRaytracingPipelineStateObject();

		bool											mInitialized;
		std::unique_ptr<D_GRAPHICS_UTILS::RayTracingStateObject> mRTSO;
	};
}
