#pragma once

#include <list>
#include <memory>

#ifndef D_CONTAINERS
#define D_CONTAINERS Darius::Core::Containers
#endif // !D_CONTAINERS

namespace Darius::Core::Containers
{
	// TODO: Use custom allocators using preallocated memory
	template<class T, class Alloc = std::allocator<T>>
	using DList = std::list<T, Alloc>;

}
