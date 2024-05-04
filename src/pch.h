
#pragma once
#define NOMINMAX

#include <winsdkver.h>
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <sdkddkver.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef __MINGW32__
#include <unknwn.h>
#endif
#include <wrl/client.h>

#include <DirectXMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <tuple>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <Libs/DirectXTK12/Inc/Keyboard.h>

#include <codecvt>

inline std::wstring STR2WSTR(const std::string& str)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.from_bytes(str);
}

inline std::string WSTR2STR(const std::wstring& wstr)
{
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;

    return converterX.to_bytes(wstr);
}

#ifdef __MINGW32__
namespace Microsoft
{
    namespace WRL
    {
        namespace Wrappers
        {
            class Event
            {
            public:
                Event() noexcept : m_handle{} {}
                explicit Event(HANDLE h) noexcept : m_handle{ h } {}
                ~Event() { if (m_handle) { ::CloseHandle(m_handle); m_handle = nullptr; } }

                void Attach(HANDLE h) noexcept
                {
                    if (h != m_handle)
                    {
                        if (m_handle) ::CloseHandle(m_handle);
                        m_handle = h;
                    }
                }

                bool IsValid() const { return m_handle != nullptr; }
                HANDLE Get() const { return m_handle; }

            private:
                HANDLE m_handle;
            };
        }
    }
}
#else
#include <wrl/event.h>
#endif
#define D_HR_CHECK(hr) Darius::Graphics::Utils::ThrowIfFailed(hr);
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
            (message);
            D_LOG_ERROR(message);
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
#include <pix3.h>
#endif

