#pragma once

#include "Graphics/AntiAliasing/FXAA.hpp"
#include "Graphics/CommandContext.hpp"
#include "Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp"

#include <Core/Serialization/Json.hpp>

#ifndef D_GRAPHICS_PP
#define D_GRAPHICS_PP Darius::Graphics::PostProcessing
#endif

namespace Darius::Graphics::PostProcessing
{

    struct PostProcessContextBuffers
    {
        // HDR Tone Mapping
        D_GRAPHICS_BUFFERS::StructuredBuffer&				ExposureBuffer;
        D_GRAPHICS_BUFFERS::ColorBuffer&                    SceneColor;
        D_GRAPHICS_BUFFERS::ColorBuffer&                    LumaBuffer;
        D_GRAPHICS_BUFFERS::ColorBuffer&                    LumaLR;
        D_GRAPHICS_BUFFERS::ByteAddressBuffer&              HistogramBuffer;
        D_GRAPHICS_BUFFERS::ColorBuffer&                    PostEffectsBuffer;

        // Bloom
        D_GRAPHICS_BUFFERS::ColorBuffer*                    BloomUAV1;
        D_GRAPHICS_BUFFERS::ColorBuffer*                    BloomUAV2;
        D_GRAPHICS_BUFFERS::ColorBuffer*                    BloomUAV3;
        D_GRAPHICS_BUFFERS::ColorBuffer*                    BloomUAV4;
        D_GRAPHICS_BUFFERS::ColorBuffer*                    BloomUAV5;

        // Fxaa
        AntiAliasing::FXAA::FXAABuffers                     FXAABuffers;

        std::wstring const&                                 JobId;
    };

    extern const float  InitialMinLog;
    extern const float  InitialMaxLog;

    void                Initialize(D_SERIALIZATION::Json const& settings);
    void                Shutdown();

    void                Render(PostProcessContextBuffers& contextBuffers, D_GRAPHICS::ComputeContext& context);

    float               GetExposure();

    // Copy the contents of the post effects buffer onto the main scene buffer
    void                CopyBackPostBuffer(D_GRAPHICS::ComputeContext& context, PostProcessContextBuffers& contextBuffers);

#ifdef _D_EDITOR
    bool                OptionsDrawer(D_SERIALIZATION::Json& options);
#endif

}
