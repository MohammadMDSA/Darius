#pragma once

#include <Renderer/CommandContext.hpp>
#include "Renderer/GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "Renderer/GraphicsUtils/Buffers/GpuBuffer.hpp"

#include <Core/Serialization/Json.hpp>

#ifndef D_GRAPHICS_PP
#define D_GRAPHICS_PP Darius::Graphics::PostProcessing
#endif

namespace Darius::Graphics::PostProcessing
{

    struct PostProcessContextBuffers
    {
        D_GRAPHICS_BUFFERS::StructuredBuffer&				ExposureBuffer;
        D_GRAPHICS_BUFFERS::ColorBuffer&                    SceneColor;
        D_GRAPHICS_BUFFERS::ColorBuffer&                    LumaBuffer;
        D_GRAPHICS_BUFFERS::ColorBuffer&                    LumaLR;
        D_GRAPHICS_BUFFERS::ByteAddressBuffer&              HistogramBuffer;
        D_GRAPHICS_BUFFERS::ColorBuffer&                    PostEffectsBuffer;
        std::wstring const&                                 JobId;
    };

    extern const float  InitialMinLog;
    extern const float  InitialMaxLog;

    void                Initialize(D_SERIALIZATION::Json const& settings);
    void                Shutdown();

    void                Render(PostProcessContextBuffers& contextBuffers);

    float               GetExposure();

    // Copy the contents of the post effects buffer onto the main scene buffer
    void                CopyBackPostBuffer(D_GRAPHICS::ComputeContext& context, PostProcessContextBuffers& contextBuffers);

#ifdef _D_EDITOR
    bool                OptionsDrawer(D_SERIALIZATION::Json& options);
#endif

}
