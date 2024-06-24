#include "pch.hpp"
#include "RendererCommon.hpp"

#include <Graphics/AntiAliasing/FXAA.hpp>

#define BUFFNAME(x) (contextName + std::wstring(L" " x))

namespace Darius::Renderer
{
	void RenderViewBuffers::Create(uint32_t width, uint32_t height, std::wstring const& contextName, bool customDepthAvailable)
	{
		SceneTexture.Create(BUFFNAME(L"Main Color"), width, height, 1, D_GRAPHICS::GetColorFormat());
		SceneDepth.Create(BUFFNAME(L"DepthStencil"), width, height, D_GRAPHICS::GetDepthFormat());
		if(customDepthAvailable)
			CustomDepth.Create(BUFFNAME(L"Custom Depth"), width, height, D_GRAPHICS::GetDepthFormat());
		SceneNormals.Create(BUFFNAME(L"Normals"), width, height, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

		// Linear Depth
		LinearDepth[0].Create(BUFFNAME(L"Linear Depth 0"), width, height, 1, DXGI_FORMAT_R16_UNORM);
		LinearDepth[1].Create(BUFFNAME(L"Linear Depth 1"), width, height, 1, DXGI_FORMAT_R16_UNORM);

		// Temporal Color 
		TemporalColor[0].Create(BUFFNAME(L"Temporal Color 0"), width, height, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
		TemporalColor[1].Create(BUFFNAME(L"Temporal Color 1"), width, height, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

		// Velocity Buffer
		VelocityBuffer.Create(BUFFNAME(L"Motion Vectors"), width, height, 1, DXGI_FORMAT_R32_UINT);

		// Divisible by 128 so that after dividing by 16, we still have multiples of 8x8 tiles.  The bloom
			// dimensions must be at least 1/4 native resolution to avoid undersampling.
			//uint32_t kBloomWidth = bufferWidth > 2560 ? Math::AlignUp(bufferWidth / 4, 128) : 640;
			//uint32_t kBloomHeight = bufferHeight > 1440 ? Math::AlignUp(bufferHeight / 4, 128) : 384;
		uint32_t bloomWidth = width > 2560 ? 1280 : 640;
		uint32_t bloomHeight = height > 1440 ? 768 : 384;

		// Post Processing Buffers
		float exposure = D_GRAPHICS_PP::GetExposure();
		ALIGN_DECL_16 float initExposure[] =
		{
			exposure, 1.0f / exposure, exposure, 0.0f,
			D_GRAPHICS_PP::InitialMinLog, D_GRAPHICS_PP::InitialMaxLog, D_GRAPHICS_PP::InitialMaxLog - D_GRAPHICS_PP::InitialMinLog, 1.0f / (D_GRAPHICS_PP::InitialMaxLog - D_GRAPHICS_PP::InitialMinLog)
		};
		ExposureBuffer.Create(BUFFNAME(L"Exposure"), 8, 4, initExposure);
		LumaLR.Create(BUFFNAME(L"Luma Buffer"), bloomWidth, bloomHeight, 1, DXGI_FORMAT_R8_UINT);
		BloomUAV1[0].Create(BUFFNAME(L"Bloom Buffer 1a"), bloomWidth, bloomHeight, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV1[1].Create(BUFFNAME(L"Bloom Buffer 1b"), bloomWidth, bloomHeight, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV2[0].Create(BUFFNAME(L"Bloom Buffer 2a"), bloomWidth / 2, bloomHeight / 2, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV2[1].Create(BUFFNAME(L"Bloom Buffer 2b"), bloomWidth / 2, bloomHeight / 2, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV3[0].Create(BUFFNAME(L"Bloom Buffer 3a"), bloomWidth / 4, bloomHeight / 4, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV3[1].Create(BUFFNAME(L"Bloom Buffer 3b"), bloomWidth / 4, bloomHeight / 4, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV4[0].Create(BUFFNAME(L"Bloom Buffer 4a"), bloomWidth / 8, bloomHeight / 8, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV4[1].Create(BUFFNAME(L"Bloom Buffer 4b"), bloomWidth / 8, bloomHeight / 8, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV5[0].Create(BUFFNAME(L"Bloom Buffer 5a"), bloomWidth / 16, bloomHeight / 16, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV5[1].Create(BUFFNAME(L"Bloom Buffer 5b"), bloomWidth / 16, bloomHeight / 16, 1, D_GRAPHICS::GetColorFormat());
		LumaBuffer.Create(BUFFNAME(L"Luminance"), width, height, 1, DXGI_FORMAT_R8_UNORM);
		HistogramBuffer.Create(BUFFNAME(L"Histogram"), 256, 4);
		PostEffectsBuffer.Create(BUFFNAME(L"Post Effects Buffer"), width, height, 1, DXGI_FORMAT_R32_UINT);

		// Ambient Occlusion Buffers
		const uint32_t bufferWidth1 = (width + 1) / 2;
		const uint32_t bufferWidth2 = (width + 3) / 4;
		const uint32_t bufferWidth3 = (width + 7) / 8;
		const uint32_t bufferWidth4 = (width + 15) / 16;
		const uint32_t bufferWidth5 = (width + 31) / 32;
		const uint32_t bufferWidth6 = (width + 63) / 64;
		const uint32_t bufferHeight1 = (height + 1) / 2;
		const uint32_t bufferHeight2 = (height + 3) / 4;
		const uint32_t bufferHeight3 = (height + 7) / 8;
		const uint32_t bufferHeight4 = (height + 15) / 16;
		const uint32_t bufferHeight5 = (height + 31) / 32;
		const uint32_t bufferHeight6 = (height + 63) / 64;
		SSAOFullScreen.Create(BUFFNAME(L"SSAO Full Res"), width, height, 1, DXGI_FORMAT_R8_UNORM);
		DepthDownsize1.Create(BUFFNAME(L"Depth Down-Sized 1"), bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R32_FLOAT);
		DepthDownsize2.Create(BUFFNAME(L"Depth Down-Sized 2"), bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R32_FLOAT);
		DepthDownsize3.Create(BUFFNAME(L"Depth Down-Sized 3"), bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R32_FLOAT);
		DepthDownsize4.Create(BUFFNAME(L"Depth Down-Sized 4"), bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R32_FLOAT);
		DepthTiled1.CreateArray(BUFFNAME(L"Depth De-Interleaved 1"), bufferWidth3, bufferHeight3, 16, DXGI_FORMAT_R16_FLOAT);
		DepthTiled2.CreateArray(BUFFNAME(L"Depth De-Interleaved 2"), bufferWidth4, bufferHeight4, 16, DXGI_FORMAT_R16_FLOAT);
		DepthTiled3.CreateArray(BUFFNAME(L"Depth De-Interleaved 3"), bufferWidth5, bufferHeight5, 16, DXGI_FORMAT_R16_FLOAT);
		DepthTiled4.CreateArray(BUFFNAME(L"Depth De-Interleaved 4"), bufferWidth6, bufferHeight6, 16, DXGI_FORMAT_R16_FLOAT);
		AOMerged1.Create(BUFFNAME(L"AO Re-Interleaved 1"), bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM);
		AOMerged2.Create(BUFFNAME(L"AO Re-Interleaved 2"), bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM);
		AOMerged3.Create(BUFFNAME(L"AO Re-Interleaved 3"), bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM);
		AOMerged4.Create(BUFFNAME(L"AO Re-Interleaved 4"), bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R8_UNORM);
		AOSmooth1.Create(BUFFNAME(L"AO Smoothed 1"), bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM);
		AOSmooth2.Create(BUFFNAME(L"AO Smoothed 2"), bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM);
		AOSmooth3.Create(BUFFNAME(L"AO Smoothed 3"), bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM);
		AOHighQuality1.Create(BUFFNAME(L"AO High Quality 1"), bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM);
		AOHighQuality2.Create(BUFFNAME(L"AO High Quality 2"), bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM);
		AOHighQuality3.Create(BUFFNAME(L"AO High Quality 3"), bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM);
		AOHighQuality4.Create(BUFFNAME(L"AO High Quality 4"), bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R8_UNORM);

		WorldPos.Create(BUFFNAME(L"World Pos"), width, height, 1u, DXGI_FORMAT_R32G32B32A32_FLOAT);
		NormalDepth.Create(BUFFNAME(L"Normal Depth"), width, height, 1u, DXGI_FORMAT_R32_UINT);

		const uint32_t kFXAAWorkSize = width * height / 4 + 128;
		FXAAWorkQueue.Create(BUFFNAME(L"FXAA Work Queue"), kFXAAWorkSize, sizeof(uint32_t));
		FXAAColorQueue.Create(BUFFNAME(L"FXAA Color Queue"), kFXAAWorkSize, sizeof(uint32_t));
	}

	void RenderViewBuffers::Destroy()
	{
		SceneDepth.Destroy();
		CustomDepth.Destroy();
		SceneTexture.Destroy();
		SceneNormals.Destroy();
		TemporalColor[0].Destroy();
		TemporalColor[1].Destroy();
		VelocityBuffer.Destroy();
		LinearDepth[0].Destroy();
		LinearDepth[1].Destroy();
		ExposureBuffer.Destroy();
		LumaBuffer.Destroy();
		LumaLR.Destroy();
		HistogramBuffer.Destroy();
		PostEffectsBuffer.Destroy();
		BloomUAV1[0].Destroy();
		BloomUAV1[1].Destroy();
		BloomUAV2[0].Destroy();
		BloomUAV2[1].Destroy();
		BloomUAV3[0].Destroy();
		BloomUAV3[1].Destroy();
		BloomUAV4[0].Destroy();
		BloomUAV4[1].Destroy();
		BloomUAV5[0].Destroy();
		BloomUAV5[1].Destroy();
		SSAOFullScreen.Destroy();
		DepthDownsize1.Destroy();
		DepthDownsize2.Destroy();
		DepthDownsize3.Destroy();
		DepthDownsize4.Destroy();
		DepthTiled1.Destroy();
		DepthTiled2.Destroy();
		DepthTiled3.Destroy();
		DepthTiled4.Destroy();
		AOMerged1.Destroy();
		AOMerged2.Destroy();
		AOMerged3.Destroy();
		AOMerged4.Destroy();
		AOSmooth1.Destroy();
		AOSmooth2.Destroy();
		AOSmooth3.Destroy();
		AOHighQuality1.Destroy();
		AOHighQuality2.Destroy();
		AOHighQuality3.Destroy();
		AOHighQuality4.Destroy();
		WorldPos.Destroy();
		NormalDepth.Destroy();
		FXAAWorkQueue.Destroy();
		FXAAColorQueue.Destroy();
	}

	D_GRAPHICS_PP::PostProcessContextBuffers RenderViewBuffers::GetPostProcessingBuffers(std::wstring const& contextName)
	{
		D_GRAPHICS_AA_FXAA::FXAABuffers fxaaBuffers = D_GRAPHICS_AA_FXAA::FXAABuffers
		{
			.SceneColorBuffer = SceneTexture,
			.PostProcessBuffer = PostEffectsBuffer,
			.WorkQueue = FXAAWorkQueue,
			.ColorQueue = FXAAColorQueue,
			.LumaBuffer = LumaBuffer
		};

		return D_GRAPHICS_PP::PostProcessContextBuffers
		{
			.ExposureBuffer = ExposureBuffer,
			.SceneColor = SceneTexture,
			.LumaBuffer = LumaBuffer,
			.LumaLR = LumaLR,
			.HistogramBuffer = HistogramBuffer,
			.PostEffectsBuffer = PostEffectsBuffer,
			.BloomUAV1 = BloomUAV1,
			.BloomUAV2 = BloomUAV2,
			.BloomUAV3 = BloomUAV3,
			.BloomUAV4 = BloomUAV4,
			.BloomUAV5 = BloomUAV5,
			.FXAABuffers = fxaaBuffers,
			.JobId = contextName
		};
	}
}
