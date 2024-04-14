#pragma once

#include "Common.hpp"

#define D_H_WARNING_DISABLE(...) \
_Pragma(D_STRINGIFY(warning(disable: __VA_ARGS__)))

#define D_H_WARNING_SCOPE_BEGIN() _Pragma(D_STRINGIFY(warning(push)))
#define D_H_WARNING_SCOPE_END() _Pragma(D_STRINGIFY(warning(pop)))