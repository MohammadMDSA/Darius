#pragma once

#include <nlohmann/json.hpp>

#ifndef D_CORE
#define D_CORE Darius::Core
#endif // !D_UTILS

namespace Darius::Core
{
	using Json = nlohmann::json;
}