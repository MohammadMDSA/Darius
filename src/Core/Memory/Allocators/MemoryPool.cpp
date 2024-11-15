#include "Core/pch.hpp"
#include "MemoryPool.hpp"

#include "Allocator.hpp"

namespace Darius::Core::Memory
{
	void ObjectPool::Init(Allocator* allocator, uint32_t poolSize, uint32_t objectSize)
	{
		mAllocator = allocator;
		mPoolSize = poolSize;
		mObjectsSize = objectSize;

		// Group allocate (object size + u32)
		size_t allocationSize = mPoolSize * (objectSize + sizeof(uint32_t));
		mMemory = reinterpret_cast<std::byte*>(mAllocator->Alloc(allocationSize, 1));
		memset(mMemory, 0, allocationSize);

		// Allocate and add free indices
		mFreeIndices = (uint32_t*)(mMemory + poolSize * mObjectsSize);
		mFreeIndicesHead = 0;

		for (uint32_t i = 0u; i < mPoolSize; i++)
		{
			mFreeIndices[i] = i;
		}

		mUsedIndices = 0;
	}

	void ObjectPool::Shutdown()
	{
		if (mFreeIndicesHead != 0)
		{
			D_LOG_ERROR("Object pool has unfreed resources");
		}

		D_ASSERT(mUsedIndices == 0);
		mAllocator->Free(mMemory);
	}

	uint32_t ObjectPool::ObtainObject()
	{
		// TODO: add bits for checking if resource is alive and use bitmasks.
		if (mFreeIndicesHead < mPoolSize)
		{
			const uint32_t freeIndex = mFreeIndices[mFreeIndicesHead++];
			++mUsedIndices;
			return freeIndex;
		}
		// Error no more resources left
		D_ASSERT_NOENTRY();

		return kInvalidIndex;
	}

	void ObjectPool::ReleaseObject(uint32_t index)
	{
		// TODO: add bits for checking if resource is alive and use bitmasks.
		mFreeIndices[--mFreeIndicesHead] = index;
		--mUsedIndices;
	}

	void ObjectPool::FreeAllObjects()
	{
		mFreeIndices = 0;
		mUsedIndices = 0;

		for (uint32_t i = 0; i < mPoolSize; i++)
		{
			mFreeIndices[i] = i;
		}
	}

	void* ObjectPool::AccessObject(uint32_t index)
	{
		if (index != kInvalidIndex)
			return &mMemory[index * mObjectsSize];

		return nullptr;
	}

	void const* ObjectPool::AccessObject(uint32_t index) const
	{
		if (index != kInvalidIndex)
			return &mMemory[index * mObjectsSize];

		return nullptr;
	}
}
