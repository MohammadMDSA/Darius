#include "Graphics/pch.hpp"
#include "LinearAllocator.hpp"
#include "Graphics/GraphicsCore.hpp"
#include "Graphics/GraphicsUtils/CommandListManager.hpp"
#include "Graphics/GraphicsDeviceManager.hpp"

#include <Core/Memory/Memory.hpp>

#include <thread>

using namespace std;

namespace Darius::Graphics::Utils::Memory
{
    LinearAllocatorType LinearAllocatorPageManager::sm_AutoType = kGpuExclusive;

    LinearAllocatorPageManager::LinearAllocatorPageManager()
    {
        mAllocationType = sm_AutoType;
        sm_AutoType = (LinearAllocatorType)(sm_AutoType + 1);
        D_ASSERT(sm_AutoType <= kNumAllocatorTypes);
    }

    LinearAllocatorPageManager LinearAllocator::sm_PageManager[2];

    LinearAllocationPage* LinearAllocatorPageManager::RequestPage()
    {
        lock_guard<mutex> LockGuard(mMutex);

        while (!mRetiredPages.empty() && D_GRAPHICS::GetCommandManager()->IsFenceComplete(mRetiredPages.front().first))
        {
            mAvailablePages.push(mRetiredPages.front().second);
            mRetiredPages.pop();
        }

        LinearAllocationPage* PagePtr = nullptr;

        if (!mAvailablePages.empty())
        {
            PagePtr = mAvailablePages.front();
            mAvailablePages.pop();
        }
        else
        {
            PagePtr = CreateNewPage();
            mPagePool.emplace_back(PagePtr);
        }

        return PagePtr;
    }

    void LinearAllocatorPageManager::DiscardPages(uint64_t FenceValue, const vector<LinearAllocationPage*>& UsedPages)
    {
        lock_guard<mutex> LockGuard(mMutex);
        for (auto iter = UsedPages.begin(); iter != UsedPages.end(); ++iter)
            mRetiredPages.push(make_pair(FenceValue, *iter));
    }

    void LinearAllocatorPageManager::FreeLargePages(uint64_t FenceValue, const vector<LinearAllocationPage*>& LargePages)
    {
        lock_guard<mutex> LockGuard(mMutex);

        while (!mDeletionQueue.empty() && D_GRAPHICS::GetCommandManager()->IsFenceComplete(mDeletionQueue.front().first))
        {
            delete mDeletionQueue.front().second;
            mDeletionQueue.pop();
        }

        for (auto iter = LargePages.begin(); iter != LargePages.end(); ++iter)
        {
            (*iter)->Unmap();
            mDeletionQueue.push(make_pair(FenceValue, *iter));
        }
    }

    LinearAllocationPage* LinearAllocatorPageManager::CreateNewPage(size_t PageSize)
    {
        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC ResourceDesc;
        ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        ResourceDesc.Alignment = 0;
        ResourceDesc.Height = 1;
        ResourceDesc.DepthOrArraySize = 1;
        ResourceDesc.MipLevels = 1;
        ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        ResourceDesc.SampleDesc.Count = 1;
        ResourceDesc.SampleDesc.Quality = 0;
        ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        D3D12_RESOURCE_STATES DefaultUsage;

        if (mAllocationType == kGpuExclusive)
        {
            HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
            ResourceDesc.Width = PageSize == 0 ? kGpuAllocatorPageSize : PageSize;
            ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            DefaultUsage = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
        }
        else
        {
            HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
            ResourceDesc.Width = PageSize == 0 ? kCpuAllocatorPageSize : PageSize;
            ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
            DefaultUsage = D3D12_RESOURCE_STATE_GENERIC_READ;
        }

        ID3D12Resource* pBuffer;
        D_HR_CHECK(D_GRAPHICS_DEVICE::GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE,
            &ResourceDesc, DefaultUsage, nullptr, IID_PPV_ARGS(&pBuffer)));

        pBuffer->SetName(L"LinearAllocator Page");

        return new LinearAllocationPage(pBuffer, DefaultUsage);
    }

    void LinearAllocator::CleanupUsedPages(uint64_t FenceID)
    {
        if (mCurPage == nullptr)
            return;

        mRetiredPages.push_back(mCurPage);
        mCurPage = nullptr;
        mCurOffset = 0;

        sm_PageManager[mAllocationType].DiscardPages(FenceID, mRetiredPages);
        mRetiredPages.clear();

        sm_PageManager[mAllocationType].FreeLargePages(FenceID, mLargePageList);
        mLargePageList.clear();
    }

    DynAlloc LinearAllocator::AllocateLargePage(size_t SizeInBytes)
    {
        LinearAllocationPage* OneOff = sm_PageManager[mAllocationType].CreateNewPage(SizeInBytes);
        mLargePageList.push_back(OneOff);

        DynAlloc ret(*OneOff, 0, SizeInBytes);
        ret.DataPtr = OneOff->mCpuVirtualAddress;
        ret.GpuAddress = OneOff->mGpuVirtualAddress;

        return ret;
    }

    DynAlloc LinearAllocator::Allocate(size_t SizeInBytes, size_t Alignment)
    {
        const size_t AlignmentMask = Alignment - 1;

        // Assert that it's a power of two.
        D_ASSERT((AlignmentMask & Alignment) == 0);

        // Align the allocation
        const size_t AlignedSize = D_MEMORY::AlignUpWithMask(SizeInBytes, AlignmentMask);

        if (AlignedSize > mPageSize)
            return AllocateLargePage(AlignedSize);

        mCurOffset = D_MEMORY::AlignUp(mCurOffset, Alignment);

        if (mCurOffset + AlignedSize > mPageSize)
        {
            D_ASSERT(mCurPage != nullptr);
            mRetiredPages.push_back(mCurPage);
            mCurPage = nullptr;
        }

        if (mCurPage == nullptr)
        {
            mCurPage = sm_PageManager[mAllocationType].RequestPage();
            mCurOffset = 0;
        }

        DynAlloc ret(*mCurPage, mCurOffset, AlignedSize);
        ret.DataPtr = (uint8_t*)mCurPage->mCpuVirtualAddress + mCurOffset;
        ret.GpuAddress = mCurPage->mGpuVirtualAddress + mCurOffset;

        mCurOffset += AlignedSize;

        return ret;
    }
}