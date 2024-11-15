#pragma once

#include "Core/StringId.hpp"
#include <unordered_map>
#include <map>
#include <memory>
#include <concurrent_unordered_map.h>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	template<typename K, typename V, typename Hash = std::hash<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
	using DUnorderedMap = std::unordered_map<K, V, Hash, std::equal_to<K>, Alloc>;

	template<typename V, typename Alloc = std::allocator<std::pair<const StringId, V>>>
	using DStringIdMap = DUnorderedMap<StringId, V, std::hash<StringId>, Alloc>;

	template<typename K, typename V, typename Comp = std::less<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
	using DMap = std::map<K, V, Comp, Alloc>;

	template<typename K, typename V, typename Hash = std::hash<K>, typename Alloc = std::allocator<std::pair<const K, V>>>
	using DConcurrentUnorderedMap = concurrency::concurrent_unordered_map<K, V, Hash, std::equal_to<K>, Alloc>;
}
