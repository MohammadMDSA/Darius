#ifdef _DEBUG
#include <iostream>
#define D_LOG_IMPL(msg, lvl)
#define D_LOG_TRACE_IMPL(msg) std::cout << "TRACE:\t" << msg << std::endl
#define D_LOG_DEBUG_IMPL(msg) std::cout  << "DEBUG:\t" << msg << std::endl
#define D_LOG_INFO_IMPL(msg) std::cout  << "INFO\t" << msg << std::endl
#define D_LOG_WARN_IMPL(msg) std::cerr  << "WARN\t" << msg << std::endl
#define D_LOG_ERROR_IMPL(msg) std::cerr  << "ERROR\t" << msg << std::endl
#define D_LOG_FATAL_IMPL(msg) std::cerr  << "FATAL\t" << msg << std::endl
#elif
#define D_LOG_IMPL(msg, lvl)
#define D_LOG_TRACE_IMPL(msg)
#define D_LOG_DEBUG_IMPL(msg)
#define D_LOG_INFO_IMPL(msg)
#define D_LOG_WARN_IMPL(msg)
#define D_LOG_ERROR_IMPL(msg)
#define D_LOG_FATAL_IMPL(msg)

#endif // _DEBUG


#define D_LOG(msg, lvl) D_LOG_IMPL(msg, lvl)
#define D_LOG_TRACE(msg) D_LOG_TRACE_IMPL(msg)
#define D_LOG_DEBUG(msg) D_LOG_DEBUG_IMPL(msg)
#define D_LOG_INFO(msg) D_LOG_INFO_IMPL(msg)
#define D_LOG_WARN(msg) D_LOG_WARN_IMPL(msg)
#define D_LOG_ERROR(msg) D_LOG_ERROR_IMPL(msg)
#define D_LOG_FATAL(msg) D_LOG_FATAL_IMPL(msg)

#ifndef _DEBUG

#define WARN_ONCE_IF( isTrue, ... ) (void)(isTrue)
#define WARN_ONCE_IF_NOT( isTrue, ... ) (void)(isTrue)
#define ERROR( msg, ... )
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

#define ERROR( ... ) \
        D_LOG_ERROR("\nError reported in " STRINGIFY_BUILTIN(__FILE__) " @ " STRINGIFY_BUILTIN(__LINE__) "\n"); \
        D_LOG_ERROR(__VA_ARGS__); \
        D_LOG_ERROR("\n");

#define DEBUGPRINT( msg, ... ) \
    D_LOG_DEBUG( msg "\n", ##__VA_ARGS__ );

#endif

#define BreakIfFailed( hr ) if (FAILED(hr)) __debugbreak()
