#include "Graphics/pch.hpp"
#include "FXAA.hpp"

#include "Graphics/CommandContext.hpp"
#include "Graphics/GraphicsUtils/RootSignature.hpp"
#include "Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp"
#include "Graphics/GraphicsUtils/Profiling/Profiling.hpp"
#include "Graphics/GraphicsUtils/Shader/Shaders.hpp"

#if _D_EDITOR
#include <imgui.h>
#endif // _D_EDITOR


using namespace D_GRAPHICS;
using namespace D_GRAPHICS_UTILS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_PROFILING;

namespace Darius::Graphics::AntiAliasing::FXAA
{

    RootSignature                           FXAARootSig;
    ComputePSO                              Pass1HdrCS(L"FXAA: Pass 1 HDR CS");
    ComputePSO                              Pass1LdrCS(L"FXAA: Pass 1 LDR CS");
    ComputePSO                              ResolveWorkCS(L"FXAA: Resolve Work CS");
    ComputePSO                              Pass2HCS(L"FXAA: Pass 2 H CS");
    ComputePSO                              Pass2VCS(L"FXAA: Pass 2 V CS");
    ComputePSO                              Pass2HDebugCS(L"FXAA: Pass 2 H Debug CS");
    ComputePSO                              Pass2VDebugCS(L"FXAA: Pass 2 V Debug CS");
    IndirectArgsBuffer                      IndirectParameters;
    ByteAddressBuffer                       WorkCounters;

    bool                                    _initialized = false;

    // Options
    bool                                    Enable;
    bool                                    DebugDraw;
    float                                   ContrastThreshold;
    float                                   SubpixelRemoval;
    bool                                    ForceOffPreComputedLuma;

	void Initialize(D_SERIALIZATION::Json const& settings)
	{
        D_ASSERT(!_initialized);

        D_H_OPTIONS_LOAD_BASIC_DEFAULT("AntiAliasing.FXAA.Enable", Enable, true);
        D_H_OPTIONS_LOAD_BASIC_DEFAULT("AntiAliasing.FXAA.DebugDraw", DebugDraw, false);
        D_H_OPTIONS_LOAD_BASIC_DEFAULT("AntiAliasing.FXAA.ContrastThreshold", ContrastThreshold, 0.175f);
        D_H_OPTIONS_LOAD_BASIC_DEFAULT("AntiAliasing.FXAA.SubpixelRemoval", SubpixelRemoval, 0.5f);
        D_H_OPTIONS_LOAD_BASIC_DEFAULT("AntiAliasing.FXAA.ForceOffPreComputedLuma", ForceOffPreComputedLuma, false);

        FXAARootSig.Reset(3, 1);
        FXAARootSig.InitStaticSampler(0, D_GRAPHICS::SamplerBilinearClampDesc);
        FXAARootSig[0].InitAsConstants(0, 7);
        FXAARootSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 0, 5);
        FXAARootSig[2].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 6);
        FXAARootSig.Finalize(L"FXAA");


#define CreatePSO(ObjName, ShaderName) \
    { \
        ObjName.SetRootSignature(FXAARootSig); \
        auto shader = D_GRAPHICS::GetShaderByName<Shaders::ComputeShader>(#ShaderName); \
        ObjName.SetComputeShader(shader); \
        ObjName.Finalize(); \
    }

        CreatePSO(ResolveWorkCS, FXAAResolveWorkQueueCS);
        if(D_GRAPHICS_DEVICE::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT())
        {
            CreatePSO(Pass1LdrCS, FXAAPass1_RGB2_CS);    // Use RGB and recompute log-luma; pre-computed luma is unavailable
            CreatePSO(Pass1HdrCS, FXAAPass1_Luma2_CS);   // Use pre-computed luma
            CreatePSO(Pass2HCS, FXAAPass2H2CS);
            CreatePSO(Pass2VCS, FXAAPass2V2CS);
            CreatePSO(Pass2HDebugCS, FXAAPass2HDebug2CS);
            CreatePSO(Pass2VDebugCS, FXAAPass2VDebug2CS);
        }
        else
        {
            CreatePSO(Pass1LdrCS, FXAAPass1_RGB_CS);     // Use RGB and recompute log-luma; pre-computed luma is unavailable
            CreatePSO(Pass1HdrCS, FXAAPass1_Luma_CS);    // Use pre-computed luma
            CreatePSO(Pass2HCS, FXAAPass2HCS);
            CreatePSO(Pass2VCS, FXAAPass2VCS);
            CreatePSO(Pass2HDebugCS, FXAAPass2HDebugCS);
            CreatePSO(Pass2VDebugCS, FXAAPass2VDebugCS);
        }
#undef CreatePSO

        __declspec(align(16)) const uint32_t initArgs[6] = {0, 1, 1, 0, 1, 1};
        IndirectParameters.Create(L"FXAA Indirect Parameters", 2, sizeof(D3D12_DISPATCH_ARGUMENTS), initArgs);
        WorkCounters.Create(L"FXAA Work Counters", 2, sizeof(uint32_t));


        GraphicsContext& InitContext = GraphicsContext::Begin();
        InitContext.TransitionResource(WorkCounters, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        InitContext.ClearUAV(WorkCounters);
        InitContext.Finish();

        _initialized = true;
	}

    void Shutdown()
    {
        D_ASSERT(_initialized);
        IndirectParameters.Destroy();
        WorkCounters.Destroy();
    }

    void Render(ComputeContext& Context, FXAABuffers& buffers, float /* toneMapperGamma */, bool bUsePreComputedLuma)
    {
        if(!Enable)
            return;

        ScopedTimer _prof(L"FXAA", Context);

        if(ForceOffPreComputedLuma)
            bUsePreComputedLuma = false;

        ColorBuffer& Target = D_GRAPHICS_DEVICE::SupportsTypedUAVLoadSupport_R11G11B10_FLOAT() ? buffers.SceneColorBuffer : buffers.PostProcessBuffer;

        Context.SetRootSignature(FXAARootSig);
        Context.SetConstants(0, 1.0f / Target.GetWidth(), 1.0f / Target.GetHeight(), (float)ContrastThreshold, (float)SubpixelRemoval);
        Context.SetConstant(0, 4, buffers.WorkQueue.GetElementCount() - 1);

        // Apply algorithm to each quarter of the screen separately to reduce maximum size of work buffers.
        uint32_t BlockWidth = Target.GetWidth() / 2;
        uint32_t BlockHeight = Target.GetHeight() / 2;

        D3D12_CPU_DESCRIPTOR_HANDLE Pass1UAVs[] =
        {
            WorkCounters.GetUAV(),
            buffers.WorkQueue.GetUAV(),
            buffers.ColorQueue.GetUAV(),
            buffers.LumaBuffer.GetUAV()
        };

        D3D12_CPU_DESCRIPTOR_HANDLE Pass1SRVs[] =
        {
            Target.GetSRV(),
            buffers.LumaBuffer.GetSRV()
        };

        for(uint32_t x = 0; x < 2; x++)
        {
            for(uint32_t y = 0; y < 2; y++)
            {
                // Pass 1
                Context.SetConstant(0, 5, x * BlockWidth);
                Context.SetConstant(0, 6, y * BlockHeight);

                // Begin by analysing the luminance buffer and setting aside high-contrast pixels in
                // work queues to be processed later.  There are horizontal edge and vertical edge work
                // queues so that the shader logic is simpler for each type of edge.
                // Counter values do not need to be reset because they are read and cleared at once.

                Context.TransitionResource(Target, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                Context.TransitionResource(buffers.WorkQueue, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                Context.TransitionResource(buffers.ColorQueue, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

                if(bUsePreComputedLuma)
                {
                    Context.SetPipelineState(Pass1HdrCS);
                    Context.TransitionResource(buffers.LumaBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                    Context.SetDynamicDescriptors(1, 0, _countof(Pass1UAVs) - 1, Pass1UAVs);
                    Context.SetDynamicDescriptors(2, 0, _countof(Pass1SRVs), Pass1SRVs);
                }
                else
                {
                    Context.SetPipelineState(Pass1LdrCS);
                    Context.TransitionResource(buffers.LumaBuffer, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                    Context.SetDynamicDescriptors(1, 0, _countof(Pass1UAVs), Pass1UAVs);
                    Context.SetDynamicDescriptors(2, 0, _countof(Pass1SRVs) - 1, Pass1SRVs);
                }

                Context.Dispatch2D(BlockWidth, BlockHeight);

                // Pass 2

                // The next phase involves converting the work queues to DispatchIndirect parameters.
                // The queues are also padded out to 64 elements to simplify the final consume logic.
                Context.SetPipelineState(ResolveWorkCS);
                Context.TransitionResource(IndirectParameters, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                Context.InsertUAVBarrier(WorkCounters);

                Context.SetDynamicDescriptor(1, 0, IndirectParameters.GetUAV());
                Context.SetDynamicDescriptor(1, 1, buffers.WorkQueue.GetUAV());
                Context.SetDynamicDescriptor(1, 2, WorkCounters.GetUAV());

                Context.Dispatch(1, 1, 1);

                Context.InsertUAVBarrier(WorkCounters);
                Context.TransitionResource(IndirectParameters, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
                Context.TransitionResource(buffers.WorkQueue, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                Context.TransitionResource(buffers.ColorQueue, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
                Context.TransitionResource(Target, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                Context.TransitionResource(buffers.LumaBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

                Context.SetDynamicDescriptor(1, 0, Target.GetUAV());
                Context.SetDynamicDescriptor(2, 0, buffers.LumaBuffer.GetSRV());
                Context.SetDynamicDescriptor(2, 1, buffers.WorkQueue.GetSRV());
                Context.SetDynamicDescriptor(2, 2, buffers.ColorQueue.GetSRV());

                // The final phase involves processing pixels on the work queues and writing them
                // back into the color buffer.  Because the two source pixels required for linearly
                // blending are held in the work queue, this does not require also sampling from
                // the target color buffer (i.e. no read/modify/write, just write.)

                Context.SetPipelineState(DebugDraw ? Pass2HDebugCS : Pass2HCS);
                Context.DispatchIndirect(IndirectParameters, 0);
                Context.SetPipelineState(DebugDraw ? Pass2VDebugCS : Pass2VCS);
                Context.DispatchIndirect(IndirectParameters, 12);

                Context.InsertUAVBarrier(Target);
            }
        }
    }


#ifdef _D_EDITOR
    bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options)
    {
        D_H_OPTION_DRAW_BEGIN();

        D_H_OPTION_DRAW_CHECKBOX("Enable FXAA", "AntiAliasing.FXAA.Enable", Enable);

        //D_H_OPTION_DRAW_CHECKBOX("Debug Draw", "AntiAliasing.FXAA.DebugDraw", DebugDraw);

        D_H_OPTION_DRAW_FLOAT_SLIDER("Contrast Threshold", "AntiAliasing.FXAA.ContrastThreshold", ContrastThreshold, 0.05f, 0.5f);

        D_H_OPTION_DRAW_FLOAT_SLIDER("Sub-Pixel Removal", "AntiAliasing.FXAA.SubpixelRemoval", SubpixelRemoval, 0.f, 1.f);
        
        D_H_OPTION_DRAW_CHECKBOX("Force Compute Luma", "AntiAliasing.FXAA.ForceOffPreComputedLuma", ForceOffPreComputedLuma);

        D_H_OPTION_DRAW_END()
    }
#endif

}