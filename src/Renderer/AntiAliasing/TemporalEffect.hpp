#pragma once

#include "Renderer/CommandContext.hpp"

#include <Core/Serialization/Json.hpp>

#ifndef D_GRAPHICS_AA_TEMPORAL
#define D_GRAPHICS_AA_TEMPORAL Darius::Graphics::AntiAliasing::TemporalEffect
#endif

namespace Darius::Graphics::AntiAliasing::TemporalEffect
{
	void		Initialize(D_SERIALIZATION::Json const& settings);
	void		Shutdown();

    void        SetEnable(bool enabled);
    bool        IsEnabled();

    // Calling once per frame
    void        Update(UINT64 frameIdx);

    // Returns whether the frame is odd or even, relevant to checkerboard rendering.
    uint32_t    GetFrameIndexMod2();

    // Jitter values are neutral at 0.5 and vary from [0, 1).  Jittering only occurs when temporal antialiasing
    // is enabled.  You can use these values to jitter your viewport or projection matrix.
    void        GetJitterOffset(float& jitterX, float& jitterY);

    void        ClearHistory(D_GRAPHICS::CommandContext& context, D_GRAPHICS_BUFFERS::ColorBuffer temporalColor[]);

    void        ResolveImage(
                    D_GRAPHICS::ComputeContext& commandContext,
                    D_GRAPHICS_BUFFERS::ColorBuffer& sceneColorBuffer,
                    D_GRAPHICS_BUFFERS::ColorBuffer& velocityBuffer,
                    D_GRAPHICS_BUFFERS::ColorBuffer temporalColor[],
                    D_GRAPHICS_BUFFERS::ColorBuffer linearDepth[]);


#ifdef _D_EDITOR
    bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif
}
