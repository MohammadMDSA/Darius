// Description:  This is a dynamic graphics memory allocator for DX12.  It's designed to work in concert
// with the CommandContext class and to do so in a thread-safe manner.  There may be many command contexts,
// each with its own linear allocators.  They act as windows into a global memory pool by reserving a
// context-local memory page.  Requesting a new page is done in a thread-safe manner by guarding accesses
// with a mutex lock.
//
// When a command context is finished, it will receive a fence ID that indicates when it's safe to reclaim
// used resources.  The CleanupUsedPages() method must be invoked at this time so that the used pages can be
// scheduled for reuse after the fence has cleared.

#pragma once

#include "Graphics/GraphicsUtils/GpuResource.hpp"

#include <Utils/Assert.hpp>

#include <vector>
#include <queue>
#include <mutex>

// Constant blocks must be multiples of 16 constants @ 16 bytes each
#define DEFAULT_ALIGN 256

#ifndef D_GRAPHICS_MEMORY
#define D_GRAPHICS_MEMORY Darius::Graphics::Utils::Memory
#endif

namespace Darius::Graphics::Utils::Memory
{
	// Various types of allocations may contain NULL pointers.  Check before dereferencing if you are unsure.
	struct DynAlloc
	{
		DynAlloc(GpuResource& BaseResource, size_t ThisOffset, size_t ThisSize) :
			Buffer(BaseResource),
			Offset(ThisOffset),
			Size(ThisSize),
			DataPtr(nullptr),
			GpuAddress(D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{}

		GpuResource& Buffer;	// The D3D buffer associated with this memory.
		size_t Offset;			// Offset from start of buffer resource
		size_t Size;			// Reserved size of this allocation
		void* DataPtr;			// The CPU-writeable address
		D3D12_GPU_VIRTUAL_ADDRESS GpuAddress;	// The GPU-visible address
	};

	class LinearAllocationPage : public GpuResource
	{
	public:
		LinearAllocationPage(ID3D12Resource* pResource, D3D12_RESOURCE_STATES Usage) : GpuResource()
		{
			mResource.Attach(pResource);
			mUsageState = Usage;
			mGpuVirtualAddress = mResource->GetGPUVirtualAddress();
			mResource->Map(0, nullptr, &mCpuVirtualAddress);
		}

		~LinearAllocationPage()
		{
			Unmap();
		}

		void Map(void)
		{
			if (mCpuVirtualAddress == nullptr)
			{
				mResource->Map(0, nullptr, &mCpuVirtualAddress);
			}
		}

		void Unmap(void)
		{
			if (mCpuVirtualAddress != nullptr)
			{
				mResource->Unmap(0, nullptr);
				mCpuVirtualAddress = nullptr;
			}
		}

		void* mCpuVirtualAddress;
		D3D12_GPU_VIRTUAL_ADDRESS mGpuVirtualAddress;
	};

	enum LinearAllocatorType
	{
		kInvalidAllocator = -1,

		kGpuExclusive = 0,		// DEFAULT   GPU-writeable (via UAV)
		kCpuWritable = 1,		// UPLOAD CPU-writeable (but write combined)

		kNumAllocatorTypes
	};

	enum
	{
		kGpuAllocatorPageSize = 0x10000,	// 64K
		kCpuAllocatorPageSize = 0x200000	// 2MB
	};

	class LinearAllocatorPageManager
	{
	public:

		LinearAllocatorPageManager();
		LinearAllocationPage* RequestPage(void);
		LinearAllocationPage* CreateNewPage(size_t PageSize = 0);

		// Discarded pages will get recycled.  This is for fixed size pages.
		void DiscardPages(uint64_t FenceID, const std::vector<LinearAllocationPage*>& Pages);

		// Freed pages will be destroyed once their fence has passed.  This is for single-use,
		// "large" pages.
		void FreeLargePages(uint64_t FenceID, const std::vector<LinearAllocationPage*>& Pages);

		void Destroy(void) { mPagePool.clear(); }

	private:

		static LinearAllocatorType sm_AutoType;

		LinearAllocatorType mAllocationType;
		std::vector<std::unique_ptr<LinearAllocationPage> > mPagePool;
		std::queue<std::pair<uint64_t, LinearAllocationPage*> > mRetiredPages;
		std::queue<std::pair<uint64_t, LinearAllocationPage*> > mDeletionQueue;
		std::queue<LinearAllocationPage*> mAvailablePages;
		std::mutex mMutex;
	};

	class LinearAllocator
	{
	public:

		LinearAllocator(LinearAllocatorType Type) : mAllocationType(Type), mPageSize(0), mCurOffset(~(size_t)0), mCurPage(nullptr)
		{
			D_ASSERT(Type > kInvalidAllocator && Type < kNumAllocatorTypes);
			mPageSize = (Type == kGpuExclusive ? kGpuAllocatorPageSize : kCpuAllocatorPageSize);
		}

		DynAlloc Allocate(size_t SizeInBytes, size_t Alignment = DEFAULT_ALIGN);

		void CleanupUsedPages(uint64_t FenceID);

		static void DestroyAll(void)
		{
			sm_PageManager[0].Destroy();
			sm_PageManager[1].Destroy();
		}

	private:

		DynAlloc AllocateLargePage(size_t SizeInBytes);

		static LinearAllocatorPageManager sm_PageManager[2];

		LinearAllocatorType mAllocationType;
		size_t mPageSize;
		size_t mCurOffset;
		LinearAllocationPage* mCurPage;
		std::vector<LinearAllocationPage*> mRetiredPages;
		std::vector<LinearAllocationPage*> mLargePageList;
	};
}