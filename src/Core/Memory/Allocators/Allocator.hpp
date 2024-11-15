#pragma once

#ifndef D_MEMORY
#define D_MEMORY Darius::Core::Memory
#endif // !D_MEMORY_ALLOC

namespace Darius::Core::Memory
{
	class Allocator
	{
	public:
		Allocator() = default;
		virtual ~Allocator() = default;
		virtual void*				Alloc(size_t size, size_t alignment) = 0;

		virtual void				Free(void* ptr) = 0;
	};
}