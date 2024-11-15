#include "Core/pch.hpp"
#include "LinearAllocator.hpp"

#include "Core/Memory/Memory.hpp"

#include <Utils/Log.hpp>

namespace Darius::Core::Memory
{

	LinearAllocator::LinearAllocator(size_t size)
	{
		Configure(size);
	}

	LinearAllocator::~LinearAllocator()
	{
		if (mMemory)
			Reset();
	}

	void LinearAllocator::Reset()
	{
		Clear();

		if (mMemory)
			DMemFree(mMemory);
	}

	void LinearAllocator::Configure(size_t size)
	{
		if (mMemory != nullptr)
			Reset();

		mMemory = reinterpret_cast<std::byte*>(DMemAlloc(size));
		mTotalSize = size;
		mAllocatedSize = 0;
	}

	void LinearAllocator::Clear()
	{
		mAllocatedSize = 0;
	}

	void* LinearAllocator::Alloc(size_t size, size_t alignment)
	{
		D_ASSERT(size > 0);
		const size_t newStart = D_MEMORY::AlignUp(mAllocatedSize, alignment);
		D_ASSERT(newStart < mTotalSize);

		const size_t newAllocatedSize = newStart + size;
		if (newAllocatedSize > mTotalSize)
		{
			D_LOG_ERROR("Linear allocator overflow");
			return nullptr;
		}

		mAllocatedSize = newAllocatedSize;
		return mMemory + newStart;
	}

	void LinearAllocator::Free(void* ptr)
	{
		D_LOG_WARN("Deallocating memory won't work with the linear allocator");
	}

}
