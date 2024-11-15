#include "Core/pch.hpp"
#include "StackAllocator.hpp"

#include "Core/Memory/Memory.hpp"

#include <Utils/Log.hpp>

namespace Darius::Core::Memory
{
	StackAllocator::StackAllocator(size_t size)
	{
		Configure(size);
	}

	StackAllocator::~StackAllocator()
	{
		if (mMemory)
		{
			Reset();
		}
	}

	void StackAllocator::Configure(size_t size)
	{
		if (mMemory != nullptr)
			Reset();

		mMemory = (std::byte*)DMemAlloc(size);
		mAllocatedSize = 0;
		mTotalSize = 0;
	}

	void* StackAllocator::Alloc(size_t size, size_t alignment)
	{
		D_ASSERT(size > 0);

		size_t newStart = AlignUp(mAllocatedSize, alignment);
		D_ASSERT(newStart < mTotalSize);
		size_t newAllocSize = newStart + size;
		if (newAllocSize > mTotalSize)
		{
			D_ASSERT_M(false, "Stack allocator size is exceeded.");
			return nullptr;
		}

		mAllocatedSize = newAllocSize;
		return mMemory + newStart;
	}

	void StackAllocator::Free(void* ptr)
	{
		D_ASSERT(ptr >= mMemory);
		D_ASSERT_M(ptr < mMemory + mTotalSize, "Out of bound free on linear allocator (outside bounds).");
		D_ASSERT_M(ptr < mMemory + mAllocatedSize, "Out of bound free on linear allocator (inside bounds).");

		size_t sizeAtPtr = (std::byte*)ptr - mMemory;

		mAllocatedSize = sizeAtPtr;
	}

	void StackAllocator::FreeMarker(size_t marker)
	{
		size_t difference = marker - mAllocatedSize;
		if (difference > 0)
			mAllocatedSize = marker;
	}

	void StackAllocator::ResetMarker()
	{
		mAllocatedSize = 0;
	}

	void StackAllocator::Reset()
	{
		DMemFree(mMemory);
		mMemory = 0;
	}

}