#pragma once

#include <unordered_set>
#include <concurrent_unordered_set.h>
#include <memory>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	// TODO: Use custom allocators using preallocated memory
	template<typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>, typename Alloc = std::allocator<T>>
	using DSet = std::unordered_set<T, Hash, Equal, Alloc>;

	// TODO: Use custom allocators using preallocated memory
	template<typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>, typename Alloc = std::allocator<T>>
	using DConcurrentSet = concurrency::concurrent_unordered_set<T, Hash, Equal, Alloc>;
}
