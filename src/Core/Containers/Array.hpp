#pragma once

#include <array>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	template<typename T, size_t Size>
	using DArray = std::array<T, Size>;
}
