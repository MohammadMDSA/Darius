#pragma once 
#include <vector>
#include <memory>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	// TODO: Use custom allocators using preallocated memory
	template<typename T, typename Alloc = std::allocator<T>>
	using DVector = std::vector<T, Alloc>;

}