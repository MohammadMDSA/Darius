#include "Renderer/pch.hpp"
#include "PostProcessing.hpp"

#include "Renderer/CommandContext.hpp"
#include "Renderer/GraphicsCore.hpp"
#include "Renderer/GraphicsUtils/PipelineState.hpp"
#include "Renderer/GraphicsUtils/Profiling/Profiling.hpp"
#include "Renderer/Resources/TextureResource.hpp"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#include <imgui.h>

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_GRAPHICS_UTILS;

namespace Darius::Graphics::PostProcessing
{

	bool                                                _initialized = false;

	// Options
	bool                                                EnableHDR;
	bool                                                EnableAdaptation;
	bool												DrawHistogram;
	float												Exposure;
	float												MinExposure;
	float												MaxExposure;
	float												TargetLuminance;
	float												AdaptationRate;

	// PSOs
	RootSignature										PostEffectRS;
	ComputePSO											ToneMapCS(L"Post Effects: Tone Map  CS");
	ComputePSO											GenerateHistogramCS(L"Post Effects: Generate Histogram CS");
	ComputePSO											DrawHistogramCS(L"Post Effects: Draw Histogram CS");
	ComputePSO											AdaptExposureCS(L"Post Effects: Adapt Exposure CS");
	ComputePSO											ExtractLumaCS(L"Post Effects: Extract Luma CS");
	ComputePSO											CopyBackPostBufferCS(L"Post Effects: Copy Back Post Buffer CS");
	ComputePSO											ApplyBloomCS(L"Post Effects: Apply Bloom CS");

	// Internal
	const float											InitialMinLog = -12.0f;
	const float											InitialMaxLog = 4.0f;
	D_CORE::Ref<TextureResource>						DefaultBlackOpaquTexture;

	// Funcs
	void ExtractLuma(ComputeContext& context, PostProcessContextBuffers& contextBuffers);
	void UpdateExposure(ComputeContext& context, PostProcessContextBuffers& contextBuffers);
	void ProcessHDR(ComputeContext&, PostProcessContextBuffers& contextBuffers);
	void ProcessLDR(ComputeContext&, PostProcessContextBuffers& contextBuffers);

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		// Initializing options
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.Enable", EnableHDR, true);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.Adaptation", EnableAdaptation, true);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.DrawHistogram", DrawHistogram, true);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.Exposure", Exposure, 2.f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.MinExposure", MinExposure, 1.0f / 64.0f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.MaxExposure", MaxExposure, 64.f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.TargetLuminance", TargetLuminance, 0.08f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.AdaptationRate", AdaptationRate, 0.05f);

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
        auto& shaderData = D_GRAPHICS::Shaders[#ShaderName]; \
        ObjName.SetComputeShader(shaderData->GetBufferPointer(), shaderData->GetBufferSize()); \
        ObjName.Finalize(); \
    }

		if (D_GRAPHICS::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
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

#undef CreatePSO

		DefaultBlackOpaquTexture = D_RESOURCE::GetResource<TextureResource>(GetDefaultGraphicsResource(DefaultResource::Texture2DBlackOpaque),
			D_CORE::CountedOwner{
				L"PostProcessin Module",
				"Subsystem Module",
				nullptr,
				0
			});

		D_ASSERT(DefaultBlackOpaquTexture.IsValid());
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

	}

	float GetExposure()
	{
		return Exposure;
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
			1.f * buffers.LumaLR.GetWidth() * buffers.LumaLR.GetHeight()
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

		if (EnableAdaptation)
			ExtractLuma(context, contextBuffers);

		if (D_GRAPHICS::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
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
			0.1f);

		context.SetConstant(0, 3, 200.f / 1000.f);
		context.SetConstant(0, 4, 1000.f);

		// Separaate out SDR result from its perceived luminance
		if (D_GRAPHICS::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
			context.SetDynamicDescriptor(1, 0, contextBuffers.SceneColor.GetUAV());
		else
		{
			context.SetDynamicDescriptor(1, 0, contextBuffers.PostEffectsBuffer.GetUAV());
			context.SetDynamicDescriptor(2, 2, contextBuffers.SceneColor.GetSRV());
		}
		context.SetDynamicDescriptor(1, 1, contextBuffers.LumaBuffer.GetUAV());

		// Read in original HDR value and blurred bloom buffer
		context.SetDynamicDescriptor(2, 0, contextBuffers.ExposureBuffer.GetSRV());
		context.SetDynamicDescriptor(2, 1, DefaultBlackOpaquTexture->GetTextureData()->GetSRV());

		context.Dispatch2D(contextBuffers.SceneColor.GetWidth(), contextBuffers.SceneColor.GetHeight());

		// Do this last so that the bright pass uses the same exposure as tone mapping
		UpdateExposure(context, contextBuffers);
	}

	void ProcessLDR(ComputeContext& context, PostProcessContextBuffers& contextBuffers)
	{
		D_PROFILING::ScopedTimer _prof(L"SDR Processing", context);

		if (!D_GRAPHICS::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
		{
			context.TransitionResource(contextBuffers.PostEffectsBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			context.TransitionResource(contextBuffers.LumaBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			// Set constants
			context.SetConstants(0,
				1.0f / contextBuffers.SceneColor.GetWidth(),
				1.0f / contextBuffers.SceneColor.GetHeight(),
				0.1f);

			// Separate out SDR result from its perceived luminance

			context.SetDynamicDescriptor(1, 0, contextBuffers.PostEffectsBuffer.GetUAV());
			context.SetDynamicDescriptor(2, 2, contextBuffers.SceneColor.GetSRV());

			context.SetDynamicDescriptor(1, 1, contextBuffers.LumaBuffer.GetUAV());

			// Read in original SDR value and blurred bloom buffer
			context.SetDynamicDescriptor(2, 0, DefaultBlackOpaquTexture->GetTextureData()->GetSRV());

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

	void Render(PostProcessContextBuffers& buffers)
	{
		auto& context = ComputeContext::Begin(buffers.JobId + L" Post Processing");

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
		if (!D_GRAPHICS::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
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

		context.Finish();
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		D_H_OPTION_DRAW_CHECKBOX("Enable HDR", "PostProcessing.HDR.Enable", EnableHDR);

		D_H_OPTION_DRAW_CHECKBOX("Adaptive Exposure", "PostProcessing.HDR.Adaptation", EnableAdaptation);

		D_H_OPTION_DRAW_CHECKBOX("Draw Histogram", "PostProcessing.HDR.DrawHistogram", DrawHistogram);

		D_H_OPTION_DRAW_FLOAT_SLIDER_EXP("Exposure", "PostProcessing.HDR.Exposure", Exposure, 1.f / 128, 128.f);

		D_H_OPTION_DRAW_FLOAT_SLIDER_EXP("Minimum Exposure", "PostProcessing.HDR.MinExposure", MinExposure, 1.f / 128, 1.f);

		D_H_OPTION_DRAW_FLOAT_SLIDER_EXP("Maximum Exposure", "PostProcessing.HDR.MaxExposure", MaxExposure, 1.f, 128.f);

		D_H_OPTION_DRAW_FLOAT_SLIDER("Target Luminance", "PostProcessing.HDR.TargetLuminance", TargetLuminance, 0.01f, 0.99f);

		D_H_OPTION_DRAW_FLOAT_SLIDER("Adaptation Rate", "PostProcessing.HDR.AdaptationRate", AdaptationRate, 0.01f, 1.f);

		D_H_OPTION_DRAW_END()

	}
#endif

}