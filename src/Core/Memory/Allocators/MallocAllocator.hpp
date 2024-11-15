#pragma once

#include "Allocator.hpp"

#include <Utils/Common.hpp>

#ifndef D_MEMORY
#define D_MEMORY Darius::Core::Memory
#endif // !D_MEMORY_ALLOC

namespace Darius::Core::Memory
{
	class MallocAllocator : public Allocator
	{
	public:
		virtual void*				Alloc(size_t size, size_t alignment) override;
		virtual void				Free(void* ptr) override;

	};
}