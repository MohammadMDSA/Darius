#include "Graphics/pch.hpp"
#include "ComputeColorLUT.hpp"

#include "Graphics/GraphicsCore.hpp"
#include "Graphics/CommandContext.hpp"
#include "Graphics/GraphicsUtils/Profiling/Profiling.hpp"
#include "Graphics/GraphicsUtils/Shader/Shaders.hpp"
#include "Graphics/GraphicsUtils/RootSignature.hpp"
#include "Graphics/GraphicsUtils/PipelineState.hpp"

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_UTILS;

namespace
{
	bool		PsoInitialized = false;
	ComputePSO	ComputeLutCS(L"Compute Tonemapper LUT");
}

void InitializePso(RootSignature const& PostEffectRS)
{
	if(PsoInitialized)
		return;
	
	// Initializing Shaders
#define CreatePSO(ObjName, ShaderName) \
    { \
        ObjName.SetRootSignature(PostEffectRS); \
        auto shader = D_GRAPHICS::GetShaderByName<D_GRAPHICS_SHADERS::ComputeShader>(#ShaderName); \
        ObjName.SetComputeShader(shader); \
        ObjName.Finalize(); \
	}

	CreatePSO(ComputeLutCS, ConstructColorLUTCS);

#undef CreatePSO

	PsoInitialized = true;
}

namespace Darius::Graphics::PostProcessing
{
	void ComputeColorLUT::ComputeLut(ToneMapperCommonConstants const& toneMapeerConstants, WorkingColorSpaceShaderParameters const& workingColorSpace, ComputeContext& context, RootSignature const& rs, bool force)
	{
		if(!LUTDirty && !force)
			return;

		InitializePso(rs);

		D_PROFILING::ScopedTimer _prof(L"Compute LUT", context);

		ShaderParams constants = Parameters.MakeShaderParams();

		if(LUT.GetUAV().ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			LUT.Create(L"Tone Mapping LUT", Parameters.LUTSize, Parameters.LUTSize, Parameters.LUTSize, 1, DXGI_FORMAT_R8G8B8A8_UNORM);

		context.TransitionResource(LUT, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		context.SetRootSignature(rs);
		context.SetPipelineState(ComputeLutCS);
		context.SetDynamicConstantBufferView(3, sizeof(ShaderParams), &constants);
		context.SetDynamicConstantBufferView(4, sizeof(ToneMapperCommonConstants), &toneMapeerConstants);
		context.SetDynamicConstantBufferView(5, sizeof(WorkingColorSpaceShaderParameters), &workingColorSpace);
		context.SetDynamicDescriptor(1, 0, LUT.GetUAV());
		context.Dispatch3D(Parameters.LUTSize, Parameters.LUTSize, Parameters.LUTSize, 8u, 8u, 8u);
		context.TransitionResource(LUT, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		LUTDirty = false;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE ComputeColorLUT::ComputeAndGetLutSrv(ToneMapperCommonConstants const& toneMapeerConstants, WorkingColorSpaceShaderParameters const& workingColorSpace, ComputeContext& context, RootSignature const& rs)
	{
		if(!LUTDirty)
			return LUT.GetSRV();

		ComputeLut(toneMapeerConstants, workingColorSpace, context, rs);

		return LUT.GetSRV();
	}


}