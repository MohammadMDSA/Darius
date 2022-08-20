#pragma once

#include <unordered_map>
#include <memory>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	// TODO: Use custom allocators using preallocated memory
	template<class K, class V, class Hash = std::hash<K>, class Alloc = std::allocator<std::pair<const K, V>>>
	using DMap = std::unordered_map<K, V, Hash, std::equal_to<K>, Alloc>;

}
