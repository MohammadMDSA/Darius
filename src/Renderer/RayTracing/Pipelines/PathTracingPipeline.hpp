#pragma once

#include "RayTracingPipeline.hpp"

#include <Graphics/GraphicsUtils/StateObject.hpp>

#ifndef D_RENDERER_RT_PIPELINE
#define D_RENDERER_RT_PIPELINE Darius::Renderer::RayTracing::Pipeline
#endif

namespace Darius::Renderer::RayTracing::Pipeline
{
	class PathTracingPipeline : public IRayTracingPipeline
	{
	public:
		PathTracingPipeline();

		virtual void				Initialize(D_SERIALIZATION::Json const& settings) override;
		virtual void				Shutdown() override;
		INLINE virtual D_GRAPHICS_UTILS::RayTracingStateObject const* GetStateObject() const override { return mRTSO.get(); };


	private:

		void						CreateRaytracingPipelineStateObject();

		bool											mInitialized;
		std::unique_ptr<D_GRAPHICS_UTILS::RayTracingStateObject> mRTSO;

		D_CORE::StringId			mRaygenShaderName;
		D_CORE::StringId			mClosestHitShaderName;
		D_CORE::StringId			mShadowClosestHitShaderName;
		D_CORE::StringId			mMissShaderName;
		D_CORE::StringId			mShadowMissShaderName;
		D_CORE::StringId			mLibName;

	};

	namespace PathTracing::GlobalRootSignatureBindings
	{
		enum slot : UINT
		{
			GlobalConstants = 0,
			GlobalRayTracingConstants,


			// 0: TLAS
			// 1: Radiance IBL
			// 2: Irradiance IBL
			// 3: -
			// 4: -
			GlobalSRVTable,


			// 10: Light Mask
			// 11: Light Data
			// 12: -
			// 13: -
			// 14: -
			GlobalLightData,

			Count
		};
	}
}
