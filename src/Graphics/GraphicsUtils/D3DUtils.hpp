#pragma once

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics::Utils
{
    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
        std::wstring const& filename,
        std::wstring const& entrypoint,
        std::wstring const& target
    );

    Microsoft::WRL::ComPtr<ID3DBlob> CompileLibrary(
        std::wstring const& filename,
        std::wstring const& target);

}