#pragma once

#include <unordered_set>
#include <memory>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	// Use custom allocators using preallocated memory
	template<typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>, typename Alloc = std::allocator<T>>
	using DSet = std::unordered_set<T, Hash, Equal, Alloc>;

}
