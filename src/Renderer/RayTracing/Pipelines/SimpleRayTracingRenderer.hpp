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

		virtual void				Initialize(D_SERIALIZATION::Json const& settings) override;
		virtual void				Shutdown() override;
		INLINE virtual D_GRAPHICS_UTILS::RayTracingStateObject const* GetStateObject() const override { return mRTSO.get(); };


	private:

		void						CreateRaytracingPipelineStateObject();

		bool											mInitialized;
		std::unique_ptr<D_GRAPHICS_UTILS::RayTracingStateObject> mRTSO;

	};

	namespace SimpleRayTracing::GlobalRootSignatureBindings
	{
		enum slot : UINT
		{
			GlobalConstants = 0,
			GlobalSRVTable,
			// 0: TLAS
			// 1: -
			// 2: -
			// 3: -
			// 4: -

			Count
		};
	}
}
