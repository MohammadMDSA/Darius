#include "Renderer/pch.hpp"
#include "TemporalEffect.hpp"

#include "Renderer/GraphicsCore.hpp"
#include "Renderer/GraphicsUtils/Profiling/Profiling.hpp"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif // _D_EDITOR


using namespace D_GRAPHICS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_GRAPHICS_UTILS;

namespace Darius::Graphics::AntiAliasing::TemporalEffect
{
	bool                                _initialized;

	// Psos
	ComputePSO                          TemporalBlendCS(L"TAA: Temporal Blend CS");
	ComputePSO                          BoundNeighborhoodCS(L"TAA: Bound Neighborhood CS");
	ComputePSO                          SharpenTAACS(L"TAA: Sharpen TAA CS");
	ComputePSO                          ResolveTAACS(L"TAA: Resolve TAA CS");

	// Vars
	uint32_t                            FrameIndex = 0;
	uint32_t                            FrameIndexMod2 = 0;
	float                               JitterX = 0.5f;
	float                               JitterY = 0.5f;
	float                               JitterDeltaX = 0.0f;
	float                               JitterDeltaY = 0.0f;

	// Options
	bool                                EnableTAA = true;
	float								Sharpness = 0.5f;
	float								TemporalMaxLerp = 1.0f;
	float								TemporalSpeedLimit = 64.0f;
	bool								TriggerReset = false;
	bool								EnableCBR = false;

	void ApplyTemporalAA(
		ComputeContext& context,
		ColorBuffer& sceneColorBuffer,
		ColorBuffer& velocityBuffer,
		ColorBuffer temporalColor[],
		ColorBuffer linearDepth[]);

	void SharpenImage(ComputeContext& context, ColorBuffer& sceneColorBuffer, ColorBuffer& temporalColor);

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		TriggerReset = true;
	
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AntiAliasing.Temporal.Enable", EnableTAA, true);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AntiAliasing.Temporal.Sharpness", Sharpness, 0.5f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AntiAliasing.Temporal.MaxLerp", TemporalMaxLerp, 1.0f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AntiAliasing.Temporal.SpeedLimit", TemporalSpeedLimit, 64.0f);

#define CreatePSO(ObjName, ShaderName) \
    { \
        ObjName.SetRootSignature(D_GRAPHICS::CommonRS); \
        auto& shaderData = D_GRAPHICS::Shaders[#ShaderName]; \
        ObjName.SetComputeShader(shaderData->GetBufferPointer(), shaderData->GetBufferSize()); \
        ObjName.Finalize(); \
    }

		CreatePSO(TemporalBlendCS, TemporalBlendCS);
		CreatePSO(BoundNeighborhoodCS, BoundNeighborhoodCS);
		CreatePSO(SharpenTAACS, SharpenTAACS);
		CreatePSO(ResolveTAACS, ResolveTAACS);


#undef CreatePSO
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}


	void SetEnable(bool enabled)
	{
		EnableTAA = enabled;
	}

	bool IsEnabled()
	{
		return EnableTAA;
	}


	void Update(UINT64 frameIdx)
	{
		EnableCBR = false;

		FrameIndex = (UINT32)frameIdx;
		FrameIndexMod2 = FrameIndex % 2;

		if (EnableTAA)
		{
			static const float Halton23[8][2] =
			{
				{ 0.0f / 8.0f, 0.0f / 9.0f }, { 4.0f / 8.0f, 3.0f / 9.0f },
				{ 2.0f / 8.0f, 6.0f / 9.0f }, { 6.0f / 8.0f, 1.0f / 9.0f },
				{ 1.0f / 8.0f, 4.0f / 9.0f }, { 5.0f / 8.0f, 7.0f / 9.0f },
				{ 3.0f / 8.0f, 2.0f / 9.0f }, { 7.0f / 8.0f, 5.0f / 9.0f }
			};

			const float* offset = nullptr;

			if (EnableCBR)
				offset = Halton23[FrameIndex % 7 + 1];
			else
				offset = Halton23[FrameIndex % 8];

			JitterDeltaX = JitterX - offset[0];
			JitterDeltaY = JitterY - offset[1];
			JitterX = offset[0];
			JitterY = offset[1];
		}
		else
		{
			JitterDeltaX = JitterX = 0.5f;
			JitterDeltaY = JitterY = 0.5f;
			JitterX = 0.5f;
			JitterY = 0.5f;
		}
	}

	uint32_t GetFrameIndexMod2()
	{
		return FrameIndexMod2;
	}

	void GetJitterOffset(float& jitterX, float& jitterY)
	{
		jitterX = JitterX;
		jitterY = JitterY;
	}

	void ClearHistory(CommandContext& context, ColorBuffer temporalColor[])
	{
		if (!EnableTAA)
			return;

		auto& gContext = context.GetGraphicsContext();

		gContext.TransitionResource(temporalColor[0], D3D12_RESOURCE_STATE_RENDER_TARGET);
		gContext.TransitionResource(temporalColor[1], D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		gContext.ClearColor(temporalColor[0]);
		gContext.ClearColor(temporalColor[1]);
	}

	void ResolveImage(
		ComputeContext& commandContext,
		ColorBuffer& sceneColorBuffer,
		ColorBuffer& velocityBuffer,
		ColorBuffer temporalColor[],
		ColorBuffer linearDepth[]
	)
	{
		D_PROFILING::ScopedTimer _prof(L"Temporal Resolve", commandContext);

		ComputeContext& context = commandContext.GetComputeContext();

		static bool sEnableTAA = false;
		if (EnableTAA != sEnableTAA || TriggerReset)
		{
			ClearHistory(context, temporalColor);
			sEnableTAA = EnableTAA;
			TriggerReset = false;
		}

		UINT32 src = FrameIndexMod2;
		UINT32 dst = src ^ 1;

		if(EnableTAA)
		{
			ApplyTemporalAA(context, sceneColorBuffer, velocityBuffer, temporalColor, linearDepth);
			SharpenImage(context, sceneColorBuffer, temporalColor[dst]);
		}
	}

	void ApplyTemporalAA(
		ComputeContext& context,
		ColorBuffer& sceneColorBuffer,
		ColorBuffer& velocityBuffer,
		ColorBuffer temporalColor[],
		ColorBuffer linearDepth[]
		)
	{
		D_PROFILING::ScopedTimer _prof(L"Resolve Image", context);

		UINT32 src = FrameIndexMod2;
		UINT32 dst = src ^ 1;

		context.SetRootSignature(D_GRAPHICS::CommonRS);
		context.SetPipelineState(TemporalBlendCS);

		ALIGN_DECL_16 struct ConstantBuffer
		{
			float RcpBufferDim[2];
			float TemporalBlendFactor;
			float RcpSeedLimiter;
			float CombinedJitter[2];
		};

		ConstantBuffer cbv =
		{
			1.f / sceneColorBuffer.GetWidth(), 1.f / sceneColorBuffer.GetHeight(),
			(float)TemporalMaxLerp,
			1.f / TemporalSpeedLimit,
			JitterDeltaX, JitterDeltaY
		};

		context.SetDynamicConstantBufferView(3, sizeof(cbv), &cbv);
		
		// Resource Transitions
		context.TransitionResource(velocityBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(sceneColorBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(temporalColor[src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(temporalColor[dst], D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.TransitionResource(linearDepth[src], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(linearDepth[dst], D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.SetDynamicDescriptor(1, 0, velocityBuffer.GetSRV());
		context.SetDynamicDescriptor(1, 1, sceneColorBuffer.GetSRV());
		context.SetDynamicDescriptor(1, 2, temporalColor[src].GetSRV());
		context.SetDynamicDescriptor(1, 3, linearDepth[src].GetSRV());
		context.SetDynamicDescriptor(1, 4, linearDepth[dst].GetSRV());
		context.SetDynamicDescriptor(2, 0, temporalColor[dst].GetUAV());

		context.Dispatch2D(sceneColorBuffer.GetWidth(), sceneColorBuffer.GetHeight(), 16, 8);
	}

	void SharpenImage(ComputeContext& context, ColorBuffer& sceneColorBuffer, ColorBuffer& temporalColor)
	{
		D_PROFILING::ScopedTimer _prof(L"Sharpend or copy image", context);

		context.TransitionResource(sceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.TransitionResource(temporalColor, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		context.SetPipelineState(Sharpness >= 0.001f ? SharpenTAACS : ResolveTAACS);
		context.SetConstants(0, 1.0f + Sharpness, 0.25f * Sharpness);
		context.SetDynamicDescriptor(1, 0, temporalColor.GetSRV());
		context.SetDynamicDescriptor(2, 0, sceneColorBuffer.GetUAV());
		context.Dispatch2D(sceneColorBuffer.GetWidth(), sceneColorBuffer.GetHeight());
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		D_H_OPTION_DRAW_CHECKBOX("Enable TAA", "AntiAliasing.Temporal.Enable", EnableTAA);

		D_H_OPTION_DRAW_FLOAT_SLIDER("Sharpness", "AntiAliasing.Temporal.Sharpness", Sharpness, 0.f, 1.f);

		D_H_OPTION_DRAW_FLOAT_SLIDER("Max Lerp", "AntiAliasing.Temporal.MaxLerp", TemporalMaxLerp, 0.f, 1.f);

		D_H_OPTION_DRAW_FLOAT_SLIDER_EXP("Speed Limit", "AntiAliasing.Temporal.SpeedLimit", TemporalSpeedLimit, 1.f, 4096.f);

		D_H_OPTION_DRAW_END()
	}
#endif

}
