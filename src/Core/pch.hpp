//
// pch.h
// Header for standard system include files.
//

#pragma once

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

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <format>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <tuple>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <Libs/DirectXTK12/Inc/Keyboard.h>

#include <locale>
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
