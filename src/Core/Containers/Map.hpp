#pragma once

#include <unordered_map>
#include <map>
#include <memory>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	// TODO: Use custom allocators using preallocated memory
	template<typename K, typename V, typename Hash = std::hash<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
	using DUnorderedMap = std::unordered_map<K, V, Hash, std::equal_to<K>, Alloc>;

	// TODO: Use custom allocators using preallocated memory
	template<typename K, typename V, typename Comp = std::less<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
	using DMap = std::map<K, V, Comp, Alloc>;
}
