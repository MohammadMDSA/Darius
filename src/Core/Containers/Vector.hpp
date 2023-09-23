#pragma once 
#include <vector>
#include <concurrent_vector.h>
#include <memory>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	// TODO: Use custom allocators using preallocated memory
	template<typename T, typename Alloc = std::allocator<T>>
	using DVector = std::vector<T, Alloc>;

	// TODO: Use custom allocators using preallocated memory
	template<typename T, typename Alloc = std::allocator<T>>
	using DConcurrentVector = concurrency::concurrent_vector<T, Alloc>;

}