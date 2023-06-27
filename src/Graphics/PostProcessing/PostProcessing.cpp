#include "Graphics/pch.hpp"
#include "PostProcessing.hpp"

#include "Graphics/CommandContext.hpp"
#include "Graphics/GraphicsCore.hpp"
#include "Graphics/GraphicsUtils/PipelineState.hpp"
#include "Graphics/GraphicsUtils/Profiling/Profiling.hpp"
#include "Graphics/GraphicsUtils/Buffers/Texture.hpp"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#include <imgui.h>

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_GRAPHICS_UTILS;

namespace Darius::Graphics::PostProcessing
{

	bool                                                _initialized = false;

	// Options HDR
	bool                                                EnableHDR;
	bool                                                EnableAdaptation;
	bool												DrawHistogram;
	float												Exposure;
	float												MinExposure;
	float												MaxExposure;
	float												TargetLuminance;
	float												AdaptationRate;

	// Options Bloom
	bool												EnableBloom;			// Whether enable
	float												BloomThreshold;			// Above this, pixels start to bloom
	float												BloomStrength;			// Controlling how much bloom is added back into the image
	float												BloomUpsampleFactor;	// Controls the focus of the blur. High values spread out more causing a haze.
	bool												HighQualityBloom;		// High quality blurs 5 octaves of bloom; low quality only blurs 3.

	// PSOs
	RootSignature										PostEffectRS;
	ComputePSO											ToneMapCS(L"Post Effects: Tone Map  CS");
	ComputePSO											GenerateHistogramCS(L"Post Effects: Generate Histogram CS");
	ComputePSO											DrawHistogramCS(L"Post Effects: Draw Histogram CS");
	ComputePSO											AdaptExposureCS(L"Post Effects: Adapt Exposure CS");
	ComputePSO											ExtractLumaCS(L"Post Effects: Extract Luma CS");
	ComputePSO											CopyBackPostBufferCS(L"Post Effects: Copy Back Post Buffer CS");
	ComputePSO											BlurCS(L"Post Effect: Blur CS");
	ComputePSO											UpsampleAndBlurCS(L"Post Effect: Upsample and Blur CS");
	ComputePSO											ApplyBloomCS(L"Post Effects: Apply Bloom CS");
	ComputePSO											DownsampleBloom2CS(L"Post Effect: Downsample Bloom 2 CS");
	ComputePSO											DownsampleBloom4CS(L"Post Effect: Downsample Bloom 4 CS");
	ComputePSO											BloomExtractAndDownsampleHdrCS(L"Post Effects: Bloom Extract and Downsample HDR CS");
	ComputePSO											BloomExtractAndDownsampleLdrCS(L"Post Effects: Bloom Extract and Downsample LDR CS");

	// Internal
	const float											InitialMinLog = -12.0f;
	const float											InitialMaxLog = 4.0f;
	Texture												DefaultBlackOpaquTexture;

	// Funcs
	void ExtractLuma(ComputeContext& context, PostProcessContextBuffers& contextBuffers);
	void UpdateExposure(ComputeContext& context, PostProcessContextBuffers& contextBuffers);
	void ProcessHDR(ComputeContext&, PostProcessContextBuffers& contextBuffers);
	void ProcessLDR(ComputeContext&, PostProcessContextBuffers& contextBuffers);
	void GenerateBloom(ComputeContext&, PostProcessContextBuffers& contextBuffers);
	void BlurBuffer(ComputeContext&, ColorBuffer buffer[2], ColorBuffer const& lowerResBuf, float upsampleBlendFactor);

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		// Initializing options
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.Enable", EnableHDR, true);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.Adaptation", EnableAdaptation, true);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.DrawHistogram", DrawHistogram, false);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.Exposure", Exposure, 2.f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.MinExposure", MinExposure, 1.0f / 64.0f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.MaxExposure", MaxExposure, 64.f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.TargetLuminance", TargetLuminance, 0.08f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.AdaptationRate", AdaptationRate, 0.05f);

		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.Bloom.Enable", EnableBloom, true);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.Bloom.Threshold", BloomThreshold, 4.f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.Bloom.Strength", BloomStrength, 0.1f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.Bloom.Scatter", BloomUpsampleFactor, 0.65f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.Bloom.HighQuality", HighQualityBloom, true);

		// Initializing Root Signature
		PostEffectRS.Reset(4, 2);
		PostEffectRS.InitStaticSampler(0, SamplerLinearClampDesc);
		PostEffectRS.InitStaticSampler(1, SamplerLinearBorderDesc);
		PostEffectRS[0].InitAsConstants(0, 5);
		PostEffectRS[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 4);
		PostEffectRS[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 4);
		PostEffectRS[3].InitAsConstantBuffer(1);
		PostEffectRS.Finalize(L"Post Effect");


		// Initializing Shaders
#define CreatePSO(ObjName, ShaderName) \
    { \
        ObjName.SetRootSignature(PostEffectRS); \
        auto shaderData = D_GRAPHICS::GetShaderByName(#ShaderName); \
        ObjName.SetComputeShader(shaderData->GetBufferPointer(), shaderData->GetBufferSize()); \
        ObjName.Finalize(); \
    }

		if (D_GRAPHICS_DEVICE::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
		{
			CreatePSO(ToneMapCS, ToneMap2CS);
			CreatePSO(ApplyBloomCS, ApplyBloom2CS);
		}
		else
		{
			CreatePSO(ToneMapCS, ToneMapCS);
			CreatePSO(ApplyBloomCS, ApplyBloomCS);
		}
		CreatePSO(GenerateHistogramCS, GenerateHistogramCS);
		CreatePSO(DrawHistogramCS, DebugDrawHistogramCS);
		CreatePSO(AdaptExposureCS, AdaptExposureCS);
		CreatePSO(ExtractLumaCS, ExtractLumaCS);
		CreatePSO(CopyBackPostBufferCS, CopyBackPostBufferCS);
		CreatePSO(DownsampleBloom2CS, DownsampleBloomCS);
		CreatePSO(DownsampleBloom4CS, DownsampleBloomAllCS);
		CreatePSO(BloomExtractAndDownsampleHdrCS, BloomExtractAndDownsampleHdrCS);
		CreatePSO(BloomExtractAndDownsampleLdrCS, BloomExtractAndDownsampleLdrCS);
		CreatePSO(BlurCS, BlurCS);
		CreatePSO(UpsampleAndBlurCS, UpsampleAndBlurCS);


#undef CreatePSO

		uint32_t blackColor = 0xFF000000;
		DefaultBlackOpaquTexture.Create2D(4, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, &blackColor);
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

	}

	float GetExposure()
	{
		return Exposure;
	}

	void BlurBuffer(ComputeContext& Context, ColorBuffer buffer[2], const ColorBuffer& lowerResBuf, float upsampleBlendFactor)
	{
		// Set the shader constants
		uint32_t bufferWidth = buffer[0].GetWidth();
		uint32_t bufferHeight = buffer[0].GetHeight();
		Context.SetConstants(0, 1.0f / bufferWidth, 1.0f / bufferHeight, upsampleBlendFactor);

		// Set the input textures and output UAV
		Context.TransitionResource(buffer[1], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		Context.SetDynamicDescriptor(1, 0, buffer[1].GetUAV());
		D3D12_CPU_DESCRIPTOR_HANDLE SRVs[2] = { buffer[0].GetSRV(), lowerResBuf.GetSRV() };
		Context.SetDynamicDescriptors(2, 0, 2, SRVs);

		// Set the shader:  upsample and blur or just blur
		Context.SetPipelineState(&buffer[0] == &lowerResBuf ? BlurCS : UpsampleAndBlurCS);

		// Dispatch the compute shader with default 8x8 thread groups
		Context.Dispatch2D(bufferWidth, bufferHeight);

		Context.TransitionResource(buffer[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	void GenerateBloom(ComputeContext& context, PostProcessContextBuffers& buffers)
	{
		D_PROFILING::ScopedTimer _prof(L"Generate Bloom", context);

		// We can generate a bloom buffer up to 1/4 smaller in each dimension without undersampling.  If only downsizing by 1/2 or less, a faster
	// shader can be used which only does one bilinear sample.

		uint32_t kBloomWidth = buffers.LumaLR.GetWidth();
		uint32_t kBloomHeight = buffers.LumaLR.GetHeight();

		// These bloom buffer dimensions were chosen for their impressive divisibility by 128 and because they are roughly 16:9.
		// The blurring algorithm is exactly 9 pixels by 9 pixels, so if the aspect ratio of each pixel is not square, the blur
		// will be oval in appearance rather than circular.  Coincidentally, they are close to 1/2 of a 720p buffer and 1/3 of
		// 1080p.  This is a common size for a bloom buffer on consoles.
		D_ASSERT_M(kBloomWidth % 16 == 0 && kBloomHeight % 16 == 0, "Bloom buffer dimensions must be multiples of 16");


		context.SetConstants(0, 1.0f / kBloomWidth, 1.0f / kBloomHeight, (float)BloomThreshold);
		context.TransitionResource(buffers.BloomUAV1[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.TransitionResource(buffers.LumaLR, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.TransitionResource(buffers.SceneColor, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(buffers.ExposureBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		{

			context.SetDynamicDescriptor(1, 0, buffers.BloomUAV1[0].GetUAV());
			context.SetDynamicDescriptor(1, 1, buffers.LumaLR.GetUAV());
			context.SetDynamicDescriptor(2, 0, buffers.SceneColor.GetSRV());
			context.SetDynamicDescriptor(2, 1, buffers.ExposureBuffer.GetSRV());

			context.SetPipelineState(EnableHDR ? BloomExtractAndDownsampleHdrCS : BloomExtractAndDownsampleLdrCS);
			context.Dispatch2D(kBloomWidth, kBloomHeight);
		}

		context.TransitionResource(buffers.BloomUAV1[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.SetDynamicDescriptor(2, 0, buffers.BloomUAV1[0].GetSRV());

		// The difference between high and low quality bloom is that high quality sums 5 octaves with a 2x frequency scale, and the low quality
		// sums 3 octaves with a 4x frequency scale.
		if (HighQualityBloom)
		{
			context.TransitionResource(buffers.BloomUAV2[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			context.TransitionResource(buffers.BloomUAV3[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			context.TransitionResource(buffers.BloomUAV4[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			context.TransitionResource(buffers.BloomUAV5[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			// Set the UAVs
			D3D12_CPU_DESCRIPTOR_HANDLE UAVs[4] = {
				buffers.BloomUAV2[0].GetUAV(), buffers.BloomUAV3[0].GetUAV(), buffers.BloomUAV4[0].GetUAV(), buffers.BloomUAV5[0].GetUAV() };
			context.SetDynamicDescriptors(1, 0, 4, UAVs);

			// Each dispatch group is 8x8 threads, but each thread reads in 2x2 source texels (bilinear filter).
			context.SetPipelineState(DownsampleBloom4CS);
			context.Dispatch2D(kBloomWidth / 2, kBloomHeight / 2);

			context.TransitionResource(buffers.BloomUAV2[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.TransitionResource(buffers.BloomUAV3[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.TransitionResource(buffers.BloomUAV4[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.TransitionResource(buffers.BloomUAV5[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			float upsampleBlendFactor = BloomUpsampleFactor;

			// Blur then upsample and blur four times
			BlurBuffer(context, buffers.BloomUAV5, buffers.BloomUAV5[0], 1.0f);
			BlurBuffer(context, buffers.BloomUAV4, buffers.BloomUAV5[1], upsampleBlendFactor);
			BlurBuffer(context, buffers.BloomUAV3, buffers.BloomUAV4[1], upsampleBlendFactor);
			BlurBuffer(context, buffers.BloomUAV2, buffers.BloomUAV3[1], upsampleBlendFactor);
			BlurBuffer(context, buffers.BloomUAV1, buffers.BloomUAV2[1], upsampleBlendFactor);
		}
		else
		{
			// Set the UAVs
			D3D12_CPU_DESCRIPTOR_HANDLE UAVs[2] = { buffers.BloomUAV3[0].GetUAV(), buffers.BloomUAV5[0].GetUAV() };
			context.SetDynamicDescriptors(1, 0, 2, UAVs);

			context.TransitionResource(buffers.BloomUAV3[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			context.TransitionResource(buffers.BloomUAV5[0], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			// Each dispatch group is 8x8 threads, but each thread reads in 2x2 source texels (bilinear filter).
			context.SetPipelineState(DownsampleBloom2CS);
			context.Dispatch2D(kBloomWidth / 2, kBloomHeight / 2);

			context.TransitionResource(buffers.BloomUAV3[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.TransitionResource(buffers.BloomUAV5[0], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			float upsampleBlendFactor = BloomUpsampleFactor * 2.0f / 3.0f;

			// Blur then upsample and blur two times
			BlurBuffer(context, buffers.BloomUAV5, buffers.BloomUAV5[0], 1.0f);
			BlurBuffer(context, buffers.BloomUAV3, buffers.BloomUAV5[1], upsampleBlendFactor);
			BlurBuffer(context, buffers.BloomUAV1, buffers.BloomUAV3[1], upsampleBlendFactor);
		}
	}

	void ExtractLuma(ComputeContext& context, PostProcessContextBuffers& contextBuffers)
	{
		D_PROFILING::ScopedTimer _prof(L"Extract Luma", context);

		context.TransitionResource(contextBuffers.LumaLR, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.TransitionResource(contextBuffers.ExposureBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(contextBuffers.SceneColor, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.SetConstants(0,
			1.f / contextBuffers.LumaLR.GetWidth(),
			1.f / contextBuffers.LumaLR.GetHeight());
		context.SetDynamicDescriptor(1, 0, contextBuffers.LumaLR.GetUAV());
		context.SetDynamicDescriptor(2, 0, contextBuffers.SceneColor.GetSRV());
		context.SetDynamicDescriptor(2, 1, contextBuffers.ExposureBuffer.GetSRV());
		context.SetPipelineState(ExtractLumaCS);
		context.Dispatch2D(contextBuffers.LumaLR.GetWidth(), contextBuffers.LumaLR.GetHeight());
	}

	void UpdateExposure(ComputeContext& context, PostProcessContextBuffers& buffers)
	{
		D_PROFILING::ScopedTimer _prof(L"Update Exposure", context);

		if (!EnableAdaptation)
		{
			ALIGN_DECL_16 float initExposure[] =
			{
				Exposure,
				1.f / Exposure,
				Exposure,
				0.f,
				InitialMinLog,
				InitialMaxLog,
				InitialMaxLog - InitialMinLog,
				1.0f / (InitialMaxLog - InitialMinLog)
			};

			context.WriteBuffer(buffers.ExposureBuffer, 0, initExposure, sizeof(initExposure));
			context.TransitionResource(buffers.ExposureBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			return;
		}

		// Generate an HDR histogram
		context.TransitionResource(buffers.HistogramBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
		context.ClearUAV(buffers.HistogramBuffer);
		context.TransitionResource(buffers.LumaLR, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.SetDynamicDescriptor(1, 0, buffers.HistogramBuffer.GetUAV());
		context.SetDynamicDescriptor(2, 0, buffers.LumaLR.GetSRV());
		context.SetPipelineState(GenerateHistogramCS);
		context.SetConstants(0, buffers.LumaLR.GetHeight());
		context.Dispatch2D(buffers.LumaLR.GetWidth(), buffers.LumaLR.GetHeight(), 16, buffers.LumaLR.GetHeight());

		ALIGN_DECL_16 struct
		{
			float TargetLuminance;
			float AdaptationRate;
			float MinExposure;
			float MaxExposure;
			uint32_t PixelCount;
		} constants =
		{
			TargetLuminance,
			AdaptationRate,
			MinExposure,
			MaxExposure,
			buffers.LumaLR.GetWidth() * buffers.LumaLR.GetHeight()
		};

		context.TransitionResource(buffers.HistogramBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(buffers.ExposureBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.SetDynamicDescriptor(1, 0, buffers.ExposureBuffer.GetUAV());
		context.SetDynamicDescriptor(2, 0, buffers.HistogramBuffer.GetSRV());
		context.SetDynamicConstantBufferView(3, sizeof(constants), &constants);
		context.SetPipelineState(AdaptExposureCS);
		context.Dispatch();
		context.TransitionResource(buffers.ExposureBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	}

	void ProcessHDR(ComputeContext& context, PostProcessContextBuffers& contextBuffers)
	{
		D_PROFILING::ScopedTimer _prof(L"HDR Tone Mapping", context);

		if (EnableBloom)
		{
			GenerateBloom(context, contextBuffers);
			context.TransitionResource(contextBuffers.BloomUAV1[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
		else if (EnableAdaptation)
			ExtractLuma(context, contextBuffers);

		if (D_GRAPHICS_DEVICE::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
			context.TransitionResource(contextBuffers.SceneColor, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		else
			context.TransitionResource(contextBuffers.PostEffectsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		context.TransitionResource(contextBuffers.LumaBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.TransitionResource(contextBuffers.ExposureBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		context.SetPipelineState(ToneMapCS);

		// Set constants
		context.SetConstants(0,
			1.f / contextBuffers.SceneColor.GetWidth(),
			1.f / contextBuffers.SceneColor.GetHeight(),
			BloomStrength);

		context.SetConstant(0, 3, 200.f / 1000.f);
		context.SetConstant(0, 4, 1000.f);

		// Separaate out SDR result from its perceived luminance
		if (D_GRAPHICS_DEVICE::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
			context.SetDynamicDescriptor(1, 0, contextBuffers.SceneColor.GetUAV());
		else
		{
			context.SetDynamicDescriptor(1, 0, contextBuffers.PostEffectsBuffer.GetUAV());
			context.SetDynamicDescriptor(2, 2, contextBuffers.SceneColor.GetSRV());
		}
		context.SetDynamicDescriptor(1, 1, contextBuffers.LumaBuffer.GetUAV());

		// Read in original HDR value and blurred bloom buffer
		context.SetDynamicDescriptor(2, 0, contextBuffers.ExposureBuffer.GetSRV());
		context.SetDynamicDescriptor(2, 1, EnableBloom ? contextBuffers.BloomUAV1[1].GetSRV() : DefaultBlackOpaquTexture.GetSRV());

		context.Dispatch2D(contextBuffers.SceneColor.GetWidth(), contextBuffers.SceneColor.GetHeight());

		// Do this last so that the bright pass uses the same exposure as tone mapping
		UpdateExposure(context, contextBuffers);
	}

	void ProcessLDR(ComputeContext& context, PostProcessContextBuffers& contextBuffers)
	{
		D_PROFILING::ScopedTimer _prof(L"SDR Processing", context);

		if (EnableBloom)
			GenerateBloom(context, contextBuffers);

		bool supportR11G11B10 = D_GRAPHICS_DEVICE::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT();

		if (EnableBloom || !supportR11G11B10)
		{
			if (supportR11G11B10)
				context.TransitionResource(contextBuffers.SceneColor, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			else
				context.TransitionResource(contextBuffers.PostEffectsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			context.TransitionResource(contextBuffers.BloomUAV1[1], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.TransitionResource(contextBuffers.LumaBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			// Set constants
			context.SetConstants(0,
				1.0f / contextBuffers.SceneColor.GetWidth(),
				1.0f / contextBuffers.SceneColor.GetHeight(),
				BloomStrength);

			// Separate out SDR result from its perceived luminance
			if (supportR11G11B10)
				context.SetDynamicDescriptor(1, 0, contextBuffers.SceneColor.GetUAV());
			else
			{
				context.SetDynamicDescriptor(1, 0, contextBuffers.PostEffectsBuffer.GetUAV());
				context.SetDynamicDescriptor(2, 2, contextBuffers.SceneColor.GetSRV());
			}
			context.SetDynamicDescriptor(1, 1, contextBuffers.LumaBuffer.GetUAV());

			// Read in original SDR value and blurred bloom buffer
			context.SetDynamicDescriptor(2, 0, EnableBloom ? contextBuffers.BloomUAV1[1].GetSRV() : DefaultBlackOpaquTexture.GetSRV());

			context.SetPipelineState(ApplyBloomCS);
			context.Dispatch2D(contextBuffers.SceneColor.GetWidth(), contextBuffers.SceneColor.GetHeight());

			context.TransitionResource(contextBuffers.LumaBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}
	}

	// Copy the contents of the post effects buffer onto the main scene buffer
	void CopyBackPostBuffer(ComputeContext& context, PostProcessContextBuffers& contextBuffers)
	{
		D_PROFILING::ScopedTimer _prof(L"Copy Post back to Scene", context);

		context.SetRootSignature(PostEffectRS);
		context.SetPipelineState(CopyBackPostBufferCS);
		context.TransitionResource(contextBuffers.SceneColor, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.TransitionResource(contextBuffers.PostEffectsBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.SetDynamicDescriptor(1, 0, contextBuffers.SceneColor.GetUAV());
		context.SetDynamicDescriptor(2, 0, contextBuffers.PostEffectsBuffer.GetSRV());
		context.Dispatch2D(contextBuffers.SceneColor.GetWidth(), contextBuffers.SceneColor.GetHeight());
	}

	void Render(PostProcessContextBuffers& buffers, D_GRAPHICS::ComputeContext& context)
	{
		context.PIXBeginEvent((buffers.JobId + L" Post Processing").c_str());

		context.SetRootSignature(PostEffectRS);

		if (EnableHDR)
			ProcessHDR(context, buffers);
		else
			ProcessLDR(context, buffers);

		bool generateLumaBuffer = EnableHDR;

		// In the case where we've been doing post processing in a separate buffer, we need to copy it
	// back to the original buffer.  It is possible to skip this step if the next shader knows to
	// do the manual format decode from UINT, but there are several code paths that need to be
	// changed, and some of them rely on texture filtering, which won't work with UINT.  Since this
	// is only to support legacy hardware and a single buffer copy isn't that big of a deal, this
	// is the most economical solution.
		if (!D_GRAPHICS_DEVICE::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
			CopyBackPostBuffer(context, buffers);

		if (DrawHistogram)
		{
			D_PROFILING::ScopedTimer _prof(L"Draw Debug Histogram", context);
			context.SetRootSignature(PostEffectRS);
			context.SetPipelineState(DrawHistogramCS);
			context.InsertUAVBarrier(buffers.SceneColor);
			context.TransitionResource(buffers.HistogramBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.TransitionResource(buffers.ExposureBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.SetDynamicDescriptor(1, 0, buffers.SceneColor.GetUAV());
			D3D12_CPU_DESCRIPTOR_HANDLE SRVs[2] = { buffers.HistogramBuffer.GetSRV(), buffers.ExposureBuffer.GetSRV() };
			context.SetDynamicDescriptors(2, 0, 2, SRVs);
			context.Dispatch(1, 32);
			context.TransitionResource(buffers.SceneColor, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		}

		context.PIXEndEvent();
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		ImGui::Text("HDR Tone Mapping");
		ImGui::Separator();

		D_H_OPTION_DRAW_CHECKBOX("Enable HDR", "PostProcessing.HDR.Enable", EnableHDR);

		D_H_OPTION_DRAW_CHECKBOX("Adaptive Exposure", "PostProcessing.HDR.Adaptation", EnableAdaptation);

		D_H_OPTION_DRAW_CHECKBOX("Draw Histogram", "PostProcessing.HDR.DrawHistogram", DrawHistogram);

		if (!EnableAdaptation)
		{
			D_H_OPTION_DRAW_FLOAT_SLIDER_EXP("Exposure", "PostProcessing.HDR.Exposure", Exposure, 1.f / 128, 128.f);
		}
		else
		{
			D_H_OPTION_DRAW_FLOAT_SLIDER_EXP("Minimum Exposure", "PostProcessing.HDR.MinExposure", MinExposure, 1.f / 128, 1.f);

			D_H_OPTION_DRAW_FLOAT_SLIDER_EXP("Maximum Exposure", "PostProcessing.HDR.MaxExposure", MaxExposure, 1.f, 128.f);

			D_H_OPTION_DRAW_FLOAT_SLIDER("Target Luminance", "PostProcessing.HDR.TargetLuminance", TargetLuminance, 0.01f, 0.99f);

			D_H_OPTION_DRAW_FLOAT_SLIDER("Adaptation Rate", "PostProcessing.HDR.AdaptationRate", AdaptationRate, 0.01f, 1.f);
		}

		ImGui::Spacing();

		ImGui::Text("Bloom");
		ImGui::Separator();

		D_H_OPTION_DRAW_CHECKBOX("Enable Bloom", "PostProcessing.Bloom.Enable", EnableBloom);

		D_H_OPTION_DRAW_FLOAT_SLIDER("Bloom Threshold", "PostProcessing.Bloom.Threshold", BloomThreshold, 0.f, 8.f);

		D_H_OPTION_DRAW_FLOAT_SLIDER("Bloom Strength", "PostProcessing.Bloom.Strength", BloomStrength, 0.f, 2.f);

		D_H_OPTION_DRAW_FLOAT_SLIDER("Bloom Scatter", "PostProcessing.Bloom.Scatter", BloomUpsampleFactor, 0.f, 1.f);

		D_H_OPTION_DRAW_CHECKBOX("Bloom High Quality", "PostProcessing.Bloom.HighQuality", HighQualityBloom);

		ImGui::Spacing();

		D_H_OPTION_DRAW_END()

	}
#endif

}