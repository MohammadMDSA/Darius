#pragma once

#include <Core/Serialization/Json.hpp>

#ifndef D_GRAPHICS_AA_FXAA
#define D_GRAPHICS_AA_FXAA Darius::Graphics::AntiAliasing::FXAA
#endif

namespace Darius::Graphics
{
    class ComputeContext;

    namespace Utils::Buffers
    {
        class ColorBuffer;
        class ByteAddressBuffer;
        class TypedBuffer;
    }
}

namespace Darius::Graphics::AntiAliasing::FXAA
{
    struct FXAABuffers
    {
        Utils::Buffers::ColorBuffer&        SceneColorBuffer;
        Utils::Buffers::ColorBuffer&        PostProcessBuffer;
        Utils::Buffers::ByteAddressBuffer&  WorkQueue;
        Utils::Buffers::TypedBuffer&        ColorQueue;
        Utils::Buffers::ColorBuffer&        LumaBuffer;
    };

    void Initialize(D_SERIALIZATION::Json const& settings);
    void Shutdown();
    void Render(ComputeContext& Context, FXAABuffers& buffers, float toneMapperGamma, bool bUsePreComputedLuma);

#ifdef _D_EDITOR
    bool OptionsDrawer(D_SERIALIZATION::Json& options);
#endif
}
