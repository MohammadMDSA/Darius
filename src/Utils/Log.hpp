#pragma once

#ifdef _DEBUG
#include <iostream>
#include <format>
#define D_LOG_IMPL(msg, lvl)
#define D_LOG_TRACE_IMPL(...) std::cout << "TRACE:\t" << __VA_ARGS__ << std::endl
#define D_LOG_DEBUG_IMPL(...) std::cout << "DEBUG:\t" << __VA_ARGS__ << std::endl
#define D_LOG_INFO_IMPL(...) std::cout << "INFO:\t" << __VA_ARGS__ << std::endl
#define D_LOG_WARN_IMPL(...) std::cerr << "WARN:\t" << __VA_ARGS__ << std::endl
#define D_LOG_ERROR_IMPL(...) std::cerr << "ERROR:\t" << __VA_ARGS__ << std::endl
#define D_LOG_FATAL_IMPL(...) std::cerr << "FATAL:\t" << __VA_ARGS__ <<  std::endl
#define D_LOG_TRACE_FMT_IMPL(fmt, ...) std::cout << std::format(fmt, __VA_ARGS__) << std::endl
#define D_LOG_DEBUG_FMT_IMPL(fmt, ...) std::cout << std::format(fmt, __VA_ARGS__) << std::endl
#define D_LOG_INFO_FMT_IMPL(fmt, ...) std::cout << std::format(fmt, __VA_ARGS__) << std::endl
#define D_LOG_WARN_FMT_IMPL(fmt, ...) std::cerr << std::format(fmt, __VA_ARGS__) << std::endl
#define D_LOG_ERROR_FMT_IMPL(fmt, ...) std::cerr << std::format(fmt, __VA_ARGS__) << std::endl
#define D_LOG_FATAL_FMT_IMPL(fmt, ...) std::cerr << std::format(fmt, __VA_ARGS__) << std::endl
#else
#define D_LOG_IMPL(msg, lvl) do{}while(0)
#define D_LOG_TRACE_IMPL(...) do{}while(0)
#define D_LOG_DEBUG_IMPL(...) do{}while(0)
#define D_LOG_INFO_IMPL(...) do{}while(0)
#define D_LOG_WARN_IMPL(...) do{}while(0)
#define D_LOG_ERROR_IMPL(...) do{}while(0)
#define D_LOG_FATAL_IMPL(...) do{}while(0)
#define D_LOG_TRACE_FMT_IMPL(...) do{}while(0)
#define D_LOG_DEBUG_FMT_IMPL(...) do{}while(0)
#define D_LOG_INFO_FMT_IMPL(...) do{}while(0)
#define D_LOG_WARN_FMT_IMPL(...) do{}while(0)
#define D_LOG_ERROR_FMT_IMPL(...) do{}while(0)
#define D_LOG_FATAL_FMT_IMPL(...) do{}while(0)

#endif // _DEBUG


#define D_LOG(msg, lvl) D_LOG_IMPL(msg, lvl)
#define D_LOG_TRACE(...) D_LOG_TRACE_IMPL(__VA_ARGS__)
#define D_LOG_DEBUG(...) D_LOG_DEBUG_IMPL(__VA_ARGS__)
#define D_LOG_INFO(...) D_LOG_INFO_IMPL(__VA_ARGS__)
#define D_LOG_WARN(...) D_LOG_WARN_IMPL(__VA_ARGS__)
#define D_LOG_ERROR(...) D_LOG_ERROR_IMPL(__VA_ARGS__)
#define D_LOG_FATAL(...) D_LOG_FATAL_IMPL(__VA_ARGS__)
#define D_LOG_TRACE_FMT(fmt, ...) D_LOG_TRACE_FMT_IMPL(fmt, __VA_ARGS__)
#define D_LOG_DEBUG_FMT(fmt, ...) D_LOG_DEBUG_FMT_IMPL(fmt, __VA_ARGS__)
#define D_LOG_INFO_FMT(fmt, ...) D_LOG_INFO_FMT_IMPL(fmt, __VA_ARGS__)
#define D_LOG_WARN_FMT(fmt, ...) D_LOG_WARN_FMT_IMPL(fmt, __VA_ARGS__)
#define D_LOG_ERROR_FMT(fmt, ...) D_LOG_ERROR_FMT_IMPL(fmt, __VA_ARGS__)
#define D_LOG_FATAL_FMT(fmt, ...) D_LOG_FATAL_FMT_IMPL(fmt, __VA_ARGS__)

#ifndef _DEBUG

#define WARN_ONCE_IF( isTrue, ... ) (void)(isTrue)
#define WARN_ONCE_IF_NOT( isTrue, ... ) (void)(isTrue)
#define DEBUGPRINT( msg, ... ) do {} while(0)

#else	// !RELEASE

#define STRINGIFY(x) #x
#define STRINGIFY_BUILTIN(x) STRINGIFY(x)

#define WARN_ONCE_IF( isTrue, ... ) \
    { \
        static bool s_TriggeredWarning = false; \
        if ((bool)(isTrue) && !s_TriggeredWarning) { \
            s_TriggeredWarning = true; \
            D_LOG_WARN("\nWarning issued in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
            D_LOG_WARN("\'" #isTrue "\' is true"); \
            D_LOG_WARN(__VA_ARGS__); \
            D_LOG_WARN("\n"); \
        } \
    }

#define WARN_ONCE_IF_NOT( isTrue, ... ) WARN_ONCE_IF(!(isTrue), __VA_ARGS__)

#define DEBUGPRINT( msg, ... ) \
    D_LOG_DEBUG( msg "\n", ##__VA_ARGS__ );

#endif

#define D_LOG_ERROR_FULL( ... ) \
        D_LOG_ERROR("\nError reported in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
        D_LOG_ERROR(__VA_ARGS__); \
        D_LOG_ERROR("\n");

#define BreakIfFailed( hr ) if (FAILED(hr)) __debugbreak()
