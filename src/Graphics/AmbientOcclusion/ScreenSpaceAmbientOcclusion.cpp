#include "Graphics/pch.hpp"
#include "ScreenSpaceAmbientOcclusion.hpp"

#include "Graphics/AntiAliasing/TemporalEffect.hpp"
#include "Graphics/GraphicsCore.hpp"
#include "Graphics/GraphicsUtils/RootSignature.hpp"
#include "Graphics/GraphicsUtils/PipelineState.hpp"
#include "Graphics/GraphicsUtils/Profiling/Profiling.hpp"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#ifdef _D_EDITOR
#include <imgui.h>
#endif

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_GRAPHICS_UTILS;
using namespace D_MATH;
using namespace D_MATH_CAMERA;

namespace Darius::Graphics::AmbientOcclusion::ScreenSpace
{

	// High quality (and better) is barely a noticeable improvement when modulated properly with ambient light.
	// However, in the debug view the quality improvement is very apparent.
	enum QualityLevel
	{
		kSsaoQualityVeryLow,
		kSsaoQualityLow,
		kSsaoQualityMedium,
		kSsaoQualityHigh,
		kSsaoQualityVeryHigh,
		kNumSsaoQualitySettings
	};

	const char* QualityLabels[kNumSsaoQualitySettings] =
	{
		"Very Low",
		"Low",
		"Medium",
		"High",
		"Very High"
	};

	bool										_initialized = false;

	// Options
	bool										Enabled = true;
	bool										DebugDraw = false;
	bool										AsyncCompute = false;
	bool										ComputeLinearZ = true;
	float										BlurTolerance = -5.f;
	float										UpsampleTolerance = -7.f;
	float										NoiseFilterTolerance = -5.f;
	float										RejectionFalloff = 2.5f;
	float										Accentuation = 2.5f;
	int											HierarchyDepth = 3;
	int											QualityLevel = kSsaoQualityHigh;

	// PSOs
	RootSignature								RootSig;
	ComputePSO									DepthPrepare1CS(L"SSAO: Depth Prepare 1 CS");
	ComputePSO									DepthPrepare2CS(L"SSAO: Depth Prepare 2 CS");
	ComputePSO									Render1CS(L"SSAO: Render 1 CS");
	ComputePSO									Render2CS(L"SSAO: Render 2 CS");
	ComputePSO									BlurUpsampleBlend[2] = { { L"SSAO: Blur Upsample Blend Low Q. CS" }, { L"SSAO: Blur Upsample Blend High Q. CS" } };	// Blend the upsampled result with the next higher resolution
	ComputePSO									BlurUpsampleFinal[2] = { { L"SSAO: Blur Upsample Final Low Q. CS" }, { L"SSAO: Blur Upsample Final High Q. CS" } };	// Don't blend the result, just upsample it
	ComputePSO									LinearizeDepthCS(L"SSAO: Linearize Depth CS");
	ComputePSO									DebugSSAOCS(L"SSAO: Debug CS");

	// Internals
	float										SampleThickness[12];	// Pre-computed sample thicknesses

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		// Initializing settings
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.Enable", Enabled, true);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.DebugDraw", DebugDraw, false);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.AsyncCompute", AsyncCompute, false);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.ComputeLinearZ", ComputeLinearZ, true);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.BlurTolerance", BlurTolerance, -5.f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.UpsampleTolerance", UpsampleTolerance, -7.f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.NoiseFilterTolerance", NoiseFilterTolerance, -3.f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.RejectionFalloff", RejectionFalloff, 2.5f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.Accentuation", Accentuation, 0.1f);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.HierarchyDepth", HierarchyDepth, 3);
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("AmbientOcclusion.SuperSample.QualityLevel", QualityLevel, kSsaoQualityHigh);

		// Initializing root signature
		RootSig.Reset(5, 2);
		RootSig.InitStaticSampler(0, SamplerLinearClampDesc);
		RootSig.InitStaticSampler(1, SamplerLinearBorderDesc);
		RootSig[0].InitAsConstants(0, 4);
		RootSig[1].InitAsConstantBuffer(1);
		RootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 5);
		RootSig[3].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 5);
		RootSig[4].InitAsBufferSRV(5);
		RootSig.Finalize(L"SSAO");


#define CreatePSO(ObjName, ShaderName) \
    { \
        ObjName.SetRootSignature(RootSig); \
        auto shaderData = D_GRAPHICS::GetShaderByName(#ShaderName); \
        ObjName.SetComputeShader(shaderData->GetBufferPointer(), shaderData->GetBufferSize()); \
        ObjName.Finalize(); \
    }

		CreatePSO(DepthPrepare1CS, AoPrepareDepthBuffers1CS);
		CreatePSO(DepthPrepare2CS, AoPrepareDepthBuffers2CS);
		CreatePSO(LinearizeDepthCS, LinearizeDepthCS);
		CreatePSO(DebugSSAOCS, DebugSSAOCS);
		CreatePSO(Render1CS, AoRender1CS);
		CreatePSO(Render2CS, AoRender2CS);

		CreatePSO(BlurUpsampleBlend[0], AoBlurUpsampleBlendOutCS);
		CreatePSO(BlurUpsampleBlend[1], AoBlurUpsamplePreMinBlendOutCS);
		CreatePSO(BlurUpsampleFinal[0], AoBlurUpsampleCS);
		CreatePSO(BlurUpsampleFinal[1], AoBlurUpsamplePreMinCS);

		SampleThickness[0] = sqrtf(1.0f - 0.2f * 0.2f);
		SampleThickness[1] = sqrtf(1.0f - 0.4f * 0.4f);
		SampleThickness[2] = sqrtf(1.0f - 0.6f * 0.6f);
		SampleThickness[3] = sqrtf(1.0f - 0.8f * 0.8f);
		SampleThickness[4] = sqrtf(1.0f - 0.2f * 0.2f - 0.2f * 0.2f);
		SampleThickness[5] = sqrtf(1.0f - 0.2f * 0.2f - 0.4f * 0.4f);
		SampleThickness[6] = sqrtf(1.0f - 0.2f * 0.2f - 0.6f * 0.6f);
		SampleThickness[7] = sqrtf(1.0f - 0.2f * 0.2f - 0.8f * 0.8f);
		SampleThickness[8] = sqrtf(1.0f - 0.4f * 0.4f - 0.4f * 0.4f);
		SampleThickness[9] = sqrtf(1.0f - 0.4f * 0.4f - 0.6f * 0.6f);
		SampleThickness[10] = sqrtf(1.0f - 0.4f * 0.4f - 0.8f * 0.8f);
		SampleThickness[11] = sqrtf(1.0f - 0.6f * 0.6f - 0.6f * 0.6f);

#undef CreatePSO
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

	void ComputeAO(ComputeContext& context, ColorBuffer& destination, ColorBuffer& depthBuffer, const float tanHalfFovH)
	{
		size_t bufferWidth = depthBuffer.GetWidth();
		size_t bufferHeight = depthBuffer.GetHeight();
		size_t arrayCount = depthBuffer.GetDepth();

		// Here we compute multipliers that convert the center depth value into (the reciprocal of)
		// sphere thicknesses at each sample location.  This assumes a maximum sample radius of 5
		// units, but since a sphere has no thickness at its extent, we don't need to sample that far
		// out.  Only samples whole integer offsets with distance less than 25 are used.  This means
		// that there is no sample at (3, 4) because its distance is exactly 25 (and has a thickness of 0.)

		// The shaders are set up to sample a circular region within a 5-pixel radius.
		const float screenSpaceDiameter = 10.f;

		// SphereDiameter = CenterDepth * ThicknessMultiplier.  This will compute the thickness of a sphere centered
		// at a specific depth.  The ellipsoid scale can stretch a sphere into an ellipsoid, which changes the
		// characteristics of the AO.
		// TanHalfFovH:  Radius of sphere in depth units if its center lies at Z = 1
		// ScreenspaceDiameter:  Diameter of sample sphere in pixel units
		// ScreenspaceDiameter / BufferWidth:  Ratio of the screen width that the sphere actually covers
		// Note about the "2.0f * ":  Diameter = 2 * Radius
		float thicknessMultiplier = 2.f * tanHalfFovH * screenSpaceDiameter / bufferWidth;

		if (arrayCount == 1)
			thicknessMultiplier *= 2.f;

		// This will transform a depth value from [0, thickness] to [0, 1].
		float inverseRangeFactor = 1.f / thicknessMultiplier;

		ALIGN_DECL_16 float ssaoCB[28];

		// The thicknesses are smaller for all off-center samples of the sphere.  Compute thicknesses relative
		// to the center sample.
		ssaoCB[0] = inverseRangeFactor / SampleThickness[0];
		ssaoCB[1] = inverseRangeFactor / SampleThickness[1];
		ssaoCB[2] = inverseRangeFactor / SampleThickness[2];
		ssaoCB[3] = inverseRangeFactor / SampleThickness[3];
		ssaoCB[4] = inverseRangeFactor / SampleThickness[4];
		ssaoCB[5] = inverseRangeFactor / SampleThickness[5];
		ssaoCB[6] = inverseRangeFactor / SampleThickness[6];
		ssaoCB[7] = inverseRangeFactor / SampleThickness[7];
		ssaoCB[8] = inverseRangeFactor / SampleThickness[8];
		ssaoCB[9] = inverseRangeFactor / SampleThickness[9];
		ssaoCB[10] = inverseRangeFactor / SampleThickness[10];
		ssaoCB[11] = inverseRangeFactor / SampleThickness[11];

		// These are the weights that are multiplied against the samples because not all samples are
		// equally important.  The farther the sample is from the center location, the less they matter.
		// We use the thickness of the sphere to determine the weight.  The scalars in front are the number
		// of samples with this weight because we sum the samples together before multiplying by the weight,
		// so as an aggregate all of those samples matter more.  After generating this table, the weights
		// are normalized.
		ssaoCB[12] = 4.0f * SampleThickness[0];	// Axial
		ssaoCB[13] = 4.0f * SampleThickness[1];	// Axial
		ssaoCB[14] = 4.0f * SampleThickness[2];	// Axial
		ssaoCB[15] = 4.0f * SampleThickness[3];	// Axial
		ssaoCB[16] = 4.0f * SampleThickness[4];	// Diagonal
		ssaoCB[17] = 8.0f * SampleThickness[5];	// L-shaped
		ssaoCB[18] = 8.0f * SampleThickness[6];	// L-shaped
		ssaoCB[19] = 8.0f * SampleThickness[7];	// L-shaped
		ssaoCB[20] = 4.0f * SampleThickness[8];	// Diagonal
		ssaoCB[21] = 8.0f * SampleThickness[9];	// L-shaped
		ssaoCB[22] = 8.0f * SampleThickness[10];	// L-shaped
		ssaoCB[23] = 4.0f * SampleThickness[11];	// Diagonal

		//#define SAMPLE_EXHAUSTIVELY

		// If we aren't using all of the samples, delete their weights before we normalize.
#ifndef SAMPLE_EXHAUSTIVELY
		ssaoCB[12] = 0.0f;
		ssaoCB[14] = 0.0f;
		ssaoCB[17] = 0.0f;
		ssaoCB[19] = 0.0f;
		ssaoCB[21] = 0.0f;
#endif

		// Normalize the weights by dividing by the sum of all weights
		float totalWeight = 0.f;
		for (int i = 12; i < 24; i++)
			totalWeight += ssaoCB[i];
		for (int i = 12; i < 24; i++)
			ssaoCB[i] /= totalWeight;

		ssaoCB[24] = 1.0f / bufferWidth;
		ssaoCB[25] = 1.0f / bufferHeight;
		ssaoCB[26] = 1.0f / -RejectionFalloff;
		ssaoCB[27] = 1.0f / (1.0f + Accentuation);

		context.SetDynamicConstantBufferView(1, sizeof(ssaoCB), ssaoCB);
		context.SetDynamicDescriptor(2, 0, destination.GetUAV());
		context.SetDynamicDescriptor(3, 0, depthBuffer.GetSRV());

		if (arrayCount == 1)
			context.Dispatch2D(bufferWidth, bufferHeight, 16, 16);
		else
			context.Dispatch3D(bufferWidth, bufferHeight, arrayCount, 8, 8, 1);
	}

	void BlurAndUpsample(ComputeContext& context, ColorBuffer& destination, ColorBuffer& hiResDepth, ColorBuffer& loResDepth, ColorBuffer* interLeavedAO, ColorBuffer* highQualityAO, ColorBuffer* hiResAO)
	{
		size_t loWidth = loResDepth.GetWidth();
		size_t loHeight = loResDepth.GetHeight();
		size_t hiWidth = hiResDepth.GetWidth();
		size_t hiHeight = hiResDepth.GetHeight();

		ComputePSO* shader = nullptr;
		if (hiResAO == nullptr)
		{
			shader = &BlurUpsampleFinal[highQualityAO == nullptr ? 0 : 1];
		}
		else
		{
			shader = &BlurUpsampleBlend[highQualityAO == nullptr ? 0 : 1];
		}
		context.SetPipelineState(*shader);

		float kBlurTolerance = 1.f - powf(10.f, BlurTolerance) * 1920.f / (float)loWidth;
		kBlurTolerance *= BlurTolerance;
		float kUpsampleTolerance = powf(10.f, UpsampleTolerance);
		float kNoiseFilterWeight = 1.f / (powf(10.f, NoiseFilterTolerance) + kUpsampleTolerance);

		ALIGN_DECL_16 float cbData[] =
		{
			1.f / loWidth,
			1.f / loHeight,
			1.f / hiWidth,
			1.f / hiHeight,
			kNoiseFilterWeight,
			1920.f / (float)loWidth,
			kBlurTolerance,
			kUpsampleTolerance
		};
		context.SetDynamicConstantBufferView(1, sizeof(cbData), cbData);

		context.TransitionResource(destination, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.TransitionResource(loResDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(hiResDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.SetDynamicDescriptor(2, 0, destination.GetUAV());
		context.SetDynamicDescriptor(3, 0, loResDepth.GetSRV());
		context.SetDynamicDescriptor(3, 1, hiResDepth.GetSRV());

		if (interLeavedAO != nullptr)
		{
			context.TransitionResource(*interLeavedAO, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.SetDynamicDescriptor(3, 2, interLeavedAO->GetSRV());
		}
		if (highQualityAO != nullptr)
		{
			context.TransitionResource(*highQualityAO, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.SetDynamicDescriptor(3, 3, highQualityAO->GetSRV());
		}
		if (hiResAO != nullptr)
		{
			context.TransitionResource(*hiResAO, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			context.SetDynamicDescriptor(3, 4, hiResAO->GetSRV());
		}

		context.Dispatch2D(hiWidth + 2, hiHeight + 2, 16, 16);
	}

	void LinearizeZ(ComputeContext& context, DepthBuffer& depth, ColorBuffer& linearDepth, float zMagic)
	{
		D_PROFILING::ScopedTimer _prof(L"Linearize Depth Buffer", context);
		context.SetRootSignature(RootSig);
		context.TransitionResource(depth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.SetConstants(0, zMagic);
		context.SetDynamicDescriptor(3, 0, depth.GetDepthSRV());
		context.TransitionResource(linearDepth, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		context.SetDynamicDescriptors(2, 0, 1, &linearDepth.GetUAV());
		context.SetPipelineState(LinearizeDepthCS);
		context.Dispatch2D(linearDepth.GetWidth(), linearDepth.GetHeight(), 16, 16);
	}

	void LinearizeZ(ComputeContext& context, DepthBuffer& depth, ColorBuffer& linearDepth, D_MATH_CAMERA::Camera const& camera)
	{
		const float nearClip = camera.GetNearClip();
		const float farClip = camera.GetFarClip();
		const float zMagic = (farClip - nearClip) / nearClip;
		LinearizeZ(context, depth, linearDepth, zMagic);
	}

	void Render(GraphicsContext& context, SSAORenderBuffers& buffers, Matrix4 const& projMat, float nearClip, float farClip)
	{
		const float zMagic = (farClip - nearClip) / nearClip;

		if (!Enabled)
		{
			D_PROFILING::ScopedTimer _prof(L"Generate SSAO", context);

			// Clear the SSAO buffer
			context.TransitionResource(buffers.SSAOFullScreen, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			context.ClearColor(buffers.SSAOFullScreen);
			context.TransitionResource(buffers.SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

			auto& compContext = context.GetComputeContext();

			if (ComputeLinearZ)
				LinearizeZ(compContext, buffers.Depth, buffers.LinearDepth, zMagic);

			if (DebugDraw)
			{
				compContext.SetRootSignature(RootSig);
				compContext.TransitionResource(buffers.SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				compContext.TransitionResource(buffers.LinearDepth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				compContext.SetDynamicDescriptors(2, 0, 1, &buffers.SceneColorBuffer.GetUAV());
				compContext.SetDynamicDescriptors(3, 0, 1, &buffers.LinearDepth.GetSRV());
				compContext.SetPipelineState(DebugSSAOCS);
				compContext.Dispatch2D(buffers.SSAOFullScreen.GetWidth(), buffers.SSAOFullScreen.GetHeight());
			}

			return;
		}

		context.TransitionResource(buffers.Depth, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(buffers.SSAOFullScreen, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		if (AsyncCompute)
		{
			// Flush the ZPrePass and wait for it on the compute queue
			D_GRAPHICS::GetCommandManager()->GetComputeQueue().StallForFence(context.Flush());
		}
		else
		{
			D_PROFILING::BeginBlock(L"Generate SSAO", &context);
		}

		auto& comContext = AsyncCompute ? ComputeContext::Begin(L"Async SSAO", true) : context.GetComputeContext();
		comContext.SetRootSignature(RootSig);

		{
			D_PROFILING::ScopedTimer _prof(L"Decompress and downsample", comContext);

			// Phase 1:  Decompress, linearize, downsample, and deinterleave the depth buffer
			comContext.SetConstants(0, zMagic);
			comContext.SetDynamicDescriptor(3, 0, buffers.Depth.GetDepthSRV());
			comContext.TransitionResource(buffers.LinearDepth, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.DepthDownsize1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.DepthTiled1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.DepthDownsize2, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.DepthTiled2, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

			D3D12_CPU_DESCRIPTOR_HANDLE DownsizeUAVs[5] = { buffers.LinearDepth.GetUAV(), buffers.DepthDownsize1.GetUAV(), buffers.DepthTiled1.GetUAV(),
				buffers.DepthDownsize2.GetUAV(), buffers.DepthTiled2.GetUAV() };
			comContext.SetDynamicDescriptors(2, 0, 5, DownsizeUAVs);

			comContext.SetPipelineState(DepthPrepare1CS);
			comContext.Dispatch2D(buffers.DepthTiled2.GetWidth() * 8, buffers.DepthTiled2.GetHeight() * 8);

			if (HierarchyDepth > 2)
			{
				comContext.SetConstants(0, 1.0f / buffers.DepthDownsize2.GetWidth(), 1.0f / buffers.DepthDownsize2.GetHeight());
				comContext.TransitionResource(buffers.DepthDownsize2, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
				comContext.TransitionResource(buffers.DepthDownsize3, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				comContext.TransitionResource(buffers.DepthTiled3, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				comContext.TransitionResource(buffers.DepthDownsize4, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				comContext.TransitionResource(buffers.DepthTiled4, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
				D3D12_CPU_DESCRIPTOR_HANDLE DownsizeAgainUAVs[4] = { buffers.DepthDownsize3.GetUAV(), buffers.DepthTiled3.GetUAV(), buffers.DepthDownsize4.GetUAV(), buffers.DepthTiled4.GetUAV() };
				comContext.SetDynamicDescriptors(2, 0, 4, DownsizeAgainUAVs);
				comContext.SetDynamicDescriptors(3, 0, 1, &buffers.DepthDownsize2.GetSRV());
				comContext.SetPipelineState(DepthPrepare2CS);
				comContext.Dispatch2D(buffers.DepthTiled4.GetWidth() * 8, buffers.DepthTiled4.GetHeight() * 8);
			}
		}

		{
			D_PROFILING::ScopedTimer _prof(L"Analyze depth volumes", comContext);

			// Load first element of projection matrix which is the cotangent of the horizontal FOV divided by 2.
			const float FovTangent = 1.0f / ((float*)&projMat)[0];

			comContext.TransitionResource(buffers.AOMerged1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.AOMerged2, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.AOMerged3, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.AOMerged4, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.AOHighQuality1, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.AOHighQuality2, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.AOHighQuality3, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.AOHighQuality4, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			comContext.TransitionResource(buffers.DepthTiled1, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			comContext.TransitionResource(buffers.DepthTiled2, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			comContext.TransitionResource(buffers.DepthTiled3, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			comContext.TransitionResource(buffers.DepthTiled4, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			comContext.TransitionResource(buffers.DepthDownsize1, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			comContext.TransitionResource(buffers.DepthDownsize2, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			comContext.TransitionResource(buffers.DepthDownsize3, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			comContext.TransitionResource(buffers.DepthDownsize4, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

			// Phase 2:  Render SSAO for each sub-tile
			if (HierarchyDepth > 3)
			{
				comContext.SetPipelineState(Render1CS);
				ComputeAO(comContext, buffers.AOMerged4, buffers.DepthTiled4, FovTangent);
				if (QualityLevel >= kSsaoQualityLow)
				{
					comContext.SetPipelineState(Render2CS);
					ComputeAO(comContext, buffers.AOHighQuality4, buffers.DepthDownsize4, FovTangent);
				}
			}
			if (HierarchyDepth > 2)
			{
				comContext.SetPipelineState(Render1CS);
				ComputeAO(comContext, buffers.AOMerged3, buffers.DepthTiled3, FovTangent);
				if (QualityLevel >= kSsaoQualityMedium)
				{
					comContext.SetPipelineState(Render2CS);
					ComputeAO(comContext, buffers.AOHighQuality3, buffers.DepthDownsize3, FovTangent);
				}
			}
			if (HierarchyDepth > 1)
			{
				comContext.SetPipelineState(Render1CS);
				ComputeAO(comContext, buffers.AOMerged2, buffers.DepthTiled2, FovTangent);
				if (QualityLevel >= kSsaoQualityHigh)
				{
					comContext.SetPipelineState(Render2CS);
					ComputeAO(comContext, buffers.AOHighQuality2, buffers.DepthDownsize2, FovTangent);
				}
			}
			{
				comContext.SetPipelineState(Render1CS);
				ComputeAO(comContext, buffers.AOMerged1, buffers.DepthTiled1, FovTangent);
				if (QualityLevel >= kSsaoQualityVeryHigh)
				{
					comContext.SetPipelineState(Render2CS);
					ComputeAO(comContext, buffers.AOHighQuality1, buffers.DepthDownsize1, FovTangent);
				}
			}

		}

		{
			D_PROFILING::ScopedTimer _prof(L"Blur and upsample", comContext);

			// Phase 4:  Iteratively blur and upsample, combining each result

			ColorBuffer* NextSRV = &buffers.AOMerged4;


			// 120 x 68 -> 240 x 135
			if (HierarchyDepth > 3)
			{
				BlurAndUpsample(comContext, buffers.AOSmooth3, buffers.DepthDownsize3, buffers.DepthDownsize4, NextSRV,
					QualityLevel >= kSsaoQualityLow ? &buffers.AOHighQuality4 : nullptr, &buffers.AOMerged3);

				NextSRV = &buffers.AOSmooth3;
			}
			else
				NextSRV = &buffers.AOMerged3;


			// 240 x 135 -> 480 x 270
			if (HierarchyDepth > 2)
			{
				BlurAndUpsample(comContext, buffers.AOSmooth2, buffers.DepthDownsize2, buffers.DepthDownsize3, NextSRV,
					QualityLevel >= kSsaoQualityMedium ? &buffers.AOHighQuality3 : nullptr, &buffers.AOMerged2);

				NextSRV = &buffers.AOSmooth2;
			}
			else
				NextSRV = &buffers.AOMerged2;

			// 480 x 270 -> 960 x 540
			if (HierarchyDepth > 1)
			{
				BlurAndUpsample(comContext, buffers.AOSmooth1, buffers.DepthDownsize1, buffers.DepthDownsize2, NextSRV,
					QualityLevel >= kSsaoQualityHigh ? &buffers.AOHighQuality2 : nullptr, &buffers.AOMerged1);

				NextSRV = &buffers.AOSmooth1;
			}
			else
				NextSRV = &buffers.AOMerged1;

			// 960 x 540 -> 1920 x 1080
			BlurAndUpsample(comContext, buffers.SSAOFullScreen, buffers.LinearDepth, buffers.DepthDownsize1, NextSRV,
				QualityLevel >= kSsaoQualityVeryHigh ? &buffers.AOHighQuality1 : nullptr, nullptr);

		} // End blur and upsample

		if (AsyncCompute)
			comContext.Finish();
		else
			D_PROFILING::EndBlock(&context);

		if (DebugDraw)
		{
			if (AsyncCompute)
			{
				auto& commandManager = *D_GRAPHICS::GetCommandManager();
				commandManager.GetGraphicsQueue().StallForProducer(
					commandManager.GetComputeQueue());
			}

			ComputeContext& CC = context.GetComputeContext();
			CC.TransitionResource(buffers.SceneColorBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
			CC.TransitionResource(buffers.SSAOFullScreen, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
			CC.SetRootSignature(RootSig);
			CC.SetPipelineState(DebugSSAOCS);
			CC.SetDynamicDescriptors(2, 0, 1, &buffers.SceneColorBuffer.GetUAV());
			CC.SetDynamicDescriptors(3, 0, 1, &buffers.SSAOFullScreen.GetSRV());
			CC.Dispatch2D(buffers.SSAOFullScreen.GetWidth(), buffers.SSAOFullScreen.GetHeight());
		}

		context.TransitionResource(buffers.SSAOFullScreen, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void Render(GraphicsContext& context, SSAORenderBuffers& buffers, D_MATH_CAMERA::Camera const& camera)
	{
		Render(context, buffers, camera.GetProjMatrix(), camera.GetNearClip(), camera.GetFarClip());
	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		D_H_OPTION_DRAW_CHECKBOX("Enable SSAO", "AmbientOcclusion.SuperSample.Enable", Enabled);
		//D_H_OPTION_DRAW_CHECKBOX("Debug Draw", "AmbientOcclusion.SuperSample.DebugDraw", DebugDraw);
		D_H_OPTION_DRAW_CHECKBOX("Async Compute", "AmbientOcclusion.SuperSample.AsyncCompute", AsyncCompute);
		D_H_OPTION_DRAW_CHECKBOX("Compute Linear Depth", "AmbientOcclusion.SuperSample.ComputeLinearZ", ComputeLinearZ);
		D_H_OPTION_DRAW_FLOAT_SLIDER("Blur Tolerance", "AmbientOcclusion.SuperSample.BlurTolerance", BlurTolerance, -8.f, -1.f);
		D_H_OPTION_DRAW_FLOAT_SLIDER("Upsample Tolerance", "AmbientOcclusion.SuperSample.UpsampleTolerance", UpsampleTolerance, -12.f, -1.f);
		D_H_OPTION_DRAW_FLOAT_SLIDER("Noise Filter Tolerance", "AmbientOcclusion.SuperSample.NoiseFilterTolerance", NoiseFilterTolerance, -8.f, 0.f);
		D_H_OPTION_DRAW_FLOAT_SLIDER("Rejection Falloff (rcp)", "AmbientOcclusion.SuperSample.RejectionFalloff", RejectionFalloff, 1.f, 10.f);
		D_H_OPTION_DRAW_FLOAT_SLIDER("Accentuation", "AmbientOcclusion.SuperSample.Accentuation", Accentuation, 0.f, 1.f);

		D_H_OPTION_DRAW_INT_SLIDER("Hierarchy Depth", "AmbientOcclusion.SuperSample.HierarchyDepth", HierarchyDepth, 1, 4);
		D_H_OPTION_DRAW_CHOICE("Quality Level", "AmbientOcclusion.SuperSample.QualityLevel", QualityLevel, QualityLabels, kNumSsaoQualitySettings);

		D_H_OPTION_DRAW_END();
	}
#endif
}

