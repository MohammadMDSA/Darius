//
// pch.h
// Header for standard system include files.
//

#pragma once

#define NOMINMAX

#define D_HR_CHECK(hr) Darius::Graphics::Utils::ThrowIfFailed(hr);
#define D_HR_FORCE(cond) D_HR_CHECK(cond ? S_OK : E_FAIL)
#define D_HR_FORCE(cond, msg) Darius::Graphics::Utils::ThrowIfFailed(cond ? S_OK : E_FAIL, msg)
#define D_HR_SUCCEEDED(hr) SUCCEEDED(hr)

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#ifdef USING_DIRECTX_HEADERS
#include <directx/dxgiformat.h>
#include <directx/d3d12.h>
#include <directx/d3dx12.h>
#include <dxguids/dxguids.h>
#else
#include <d3d12.h>

#include "d3dx12.h"
#endif

#include <dxgi1_6.h>

#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include <Core/pch.hpp>
#include <ResourceManager/pch.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
#include <Scene/pch.hpp>
#include <Utils/Log.hpp>

#undef min
#undef max

namespace Darius::Graphics::Utils
{
    // Helper class for COM exceptions
    class com_exception : public std::exception
    {
    public:
        com_exception(HRESULT hr) noexcept : result(hr) {}

        const char* what() const noexcept override
        {
            static char s_str[64] = {};
            sprintf_s(s_str, "Failure with HRESULT of %08X", static_cast<unsigned int>(result));
            return s_str;
        }

    private:
        HRESULT result;
    };

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            auto exp = com_exception(hr);
            auto message = exp.what();
            DebugBreak();
            D_LOG_ERROR(message);
            throw exp;
        }
    }

    // Helper utility converts D3D API failures into exceptions.
    inline void ThrowIfFailed(HRESULT hr, const wchar_t* msg)
    {
        if (FAILED(hr))
        {
            auto exp = com_exception(hr);
            auto message = exp.what();
            DebugBreak();
            D_LOG_ERROR(message);
            OutputDebugStringW(msg);
            throw exp;
        }
    }
}

#ifdef __MINGW32__
constexpr UINT PIX_COLOR_DEFAULT = 0;

inline void PIXBeginEvent(UINT64, PCWSTR) {}

template<typename T>
inline void PIXBeginEvent(T*, UINT64, PCWSTR) {}

inline void PIXEndEvent() {}

template<typename T>
inline void PIXEndEvent(T*) {}
#else
// To use graphics and CPU markup events with the latest version of PIX, change this to include <pix3.h>
// then add the NuGet package WinPixEventRuntime to the project.
#include <pix3.h>
#endif

