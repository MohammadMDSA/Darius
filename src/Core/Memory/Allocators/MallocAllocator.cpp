#pragma once

#include "Core/pch.hpp"
#include "MallocAllocator.hpp"

#include "Core/Memory/Memory.hpp"

#include <Utils/Log.hpp>

#ifndef D_MEMORY
#define D_MEMORY Darius::Core::Memory
#endif // !D_MEMORY_ALLOC

namespace Darius::Core::Memory
{
	void* MallocAllocator::Alloc(size_t size, size_t alignment)
	{
		return DMemAlloc(size);
	}

	void MallocAllocator::Free(void* ptr)
	{
		DMemFree(ptr);
	}
}