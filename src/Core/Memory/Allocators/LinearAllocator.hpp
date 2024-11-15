#pragma once

#include "Allocator.hpp"

#include <Utils/Common.hpp>

#ifndef D_MEMORY
#define D_MEMORY Darius::Core::Memory
#endif // !D_MEMORY_ALLOC

namespace Darius::Core::Memory
{
	class LinearAllocator : public Allocator
	{
	public:
		LinearAllocator(size_t size);
		virtual ~LinearAllocator();

		// Deletes the allocated memory for the allocator
		void						Reset();
		void						Configure(size_t size);

		// Resets the allocation
		void						Clear();

		virtual void*				Alloc(size_t size, size_t alignment) override;
		virtual void				Free(void* ptr) override;

	private:
		std::byte*					mMemory = nullptr;
		size_t						mTotalSize = 0;
		size_t						mAllocatedSize = 0;
	};

}
