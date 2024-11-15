#pragma once

#include "Allocator.hpp"

#include <Utils/Common.hpp>

#ifndef D_MEMORY
#define D_MEMORY Darius::Core::Memory
#endif // !D_MEMORY_ALLOC

namespace Darius::Core::Memory
{
	class StackAllocator : public Allocator
	{
	public:

		StackAllocator(size_t size);
		virtual ~StackAllocator();

		void					Configure(size_t size);

		virtual void*			Alloc(size_t size, size_t alignment) override;
		virtual void			Free(void* ptr) override;

		INLINE size_t			GetMarker() const { return mAllocatedSize; }
		void					FreeMarker(size_t marker);
		void					ResetMarker();

		void					Reset();

		INLINE size_t			GetTotalSize() const { return mTotalSize; }

	private:

		std::byte*				mMemory = nullptr;
		size_t					mTotalSize = 0;
		size_t					mAllocatedSize = 0;
	};
}