#pragma once

#include <Core/Containers/Vector.hpp>

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics::Utils
{
    namespace Shaders
    {
        struct ShaderMacroContainer;
    }

    Microsoft::WRL::ComPtr<ID3DBlob> CompileShader(
        std::wstring const& filename,
        std::wstring const& entrypoint,
        std::wstring const& target,
        void const* shaderCode,
        size_t shaderCodeSize,
        D_CONTAINERS::DVector<std::wstring> const& defines,
        D_CONTAINERS::DVector<std::wstring> const& includes,
        ID3D12ShaderReflection** reflectionData,
        ID3D12LibraryReflection** libraryReflectionData,
        std::string& compileLog,
        bool forceDisableOptimization
    );

    Microsoft::WRL::ComPtr<ID3DBlob> CompileLibrary(
        std::wstring const& filename,
        std::wstring const& target);

}