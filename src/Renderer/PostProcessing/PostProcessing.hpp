#pragma once

#include <Core/Serialization/Json.hpp>

#ifndef D_GRAPHICS_PP
#define D_GRAPHICS_PP Darius::Graphics::PostProcessing
#endif

namespace Darius::Graphics::PostProcessing
{

    void                Initialize(D_SERIALIZATION::Json const& settings);
    void                Shutdown();

    void                Render();


#ifdef _D_EDITOR
    bool                OptionsDrawer(D_SERIALIZATION::Json& options);
#endif

}
