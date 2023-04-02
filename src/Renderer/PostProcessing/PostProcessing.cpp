#include "Renderer/pch.hpp"
#include "PostProcessing.hpp"

#include "Renderer/CommandContext.hpp"
#include "Renderer/GraphicsCore.hpp"
#include "Renderer/GraphicsUtils/PipelineState.hpp"

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#include <imgui.h>

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_UTILS;

namespace Darius::Graphics::PostProcessing
{

	bool                                                _initialized = false;

	// Options
	bool                                                EnableHDR;

	// PSOs
	RootSignature										PostEffectRS;

	// Internal

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		// Initializing options
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("PostProcessing.HDR.Enable", EnableHDR, true);

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



#undef CreatePSO
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

	void Render()
	{

	}

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
	{
		D_H_OPTION_DRAW_BEGIN();

		D_H_OPTION_DRAW_CHECKBOX("Enable HDR", "PostProcessing.HDR.Enable", EnableHDR);

		D_H_OPTION_DRAW_END()

	}
#endif

}