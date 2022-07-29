//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "Renderer/pch.hpp"
#include "DynamicDescriptorHeap.hpp"
#include "Renderer/CommandContext.hpp"
#include "Renderer/GraphicsCore.hpp"
#include "Renderer/RenderDeviceManager.hpp"
#include "Renderer/GraphicsUtils/CommandListManager.hpp"
#include "Renderer/GraphicsUtils/RootSignature.hpp"

#include <Utils/Assert.hpp>

using namespace D_GRAPHICS_UTILS;
using namespace Darius::Graphics;


namespace Darius::Graphics::Utils::Memory
{
    //
    // DynamicDescriptorHeap Implementation
    //

    std::mutex DynamicDescriptorHeap::sm_Mutex;
    std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> DynamicDescriptorHeap::sm_DescriptorHeapPool[2];
    std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> DynamicDescriptorHeap::sm_RetiredDescriptorHeaps[2];
    std::queue<ID3D12DescriptorHeap*> DynamicDescriptorHeap::sm_AvailableDescriptorHeaps[2];

    ID3D12DescriptorHeap* DynamicDescriptorHeap::RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE HeapType)
    {
        std::lock_guard<std::mutex> LockGuard(sm_Mutex);

        uint32_t idx = HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 1 : 0;

        while (!sm_RetiredDescriptorHeaps[idx].empty() && D_GRAPHICS::GetCommandManager()->IsFenceComplete(sm_RetiredDescriptorHeaps[idx].front().first))
        {
            sm_AvailableDescriptorHeaps[idx].push(sm_RetiredDescriptorHeaps[idx].front().second);
            sm_RetiredDescriptorHeaps[idx].pop();
        }

        if (!sm_AvailableDescriptorHeaps[idx].empty())
        {
            ID3D12DescriptorHeap* HeapPtr = sm_AvailableDescriptorHeaps[idx].front();
            sm_AvailableDescriptorHeaps[idx].pop();
            return HeapPtr;
        }
        else
        {
            D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
            HeapDesc.Type = HeapType;
            HeapDesc.NumDescriptors = kNumDescriptorsPerHeap;
            HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            HeapDesc.NodeMask = 1;
            Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> HeapPtr;
            D_HR_CHECK(D_RENDERER_DEVICE::GetDevice()->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&HeapPtr)));
            sm_DescriptorHeapPool[idx].emplace_back(HeapPtr);
            return HeapPtr.Get();
        }
    }

    void DynamicDescriptorHeap::DiscardDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint64_t FenceValue, const std::vector<ID3D12DescriptorHeap*>& UsedHeaps)
    {
        uint32_t idx = HeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? 1 : 0;
        std::lock_guard<std::mutex> LockGuard(sm_Mutex);
        for (auto iter = UsedHeaps.begin(); iter != UsedHeaps.end(); ++iter)
            sm_RetiredDescriptorHeaps[idx].push(std::make_pair(FenceValue, *iter));
    }

    void DynamicDescriptorHeap::RetireCurrentHeap(void)
    {
        // Don't retire unused heaps.
        if (mCurrentOffset == 0)
        {
            D_ASSERT(mCurrentHeapPtr == nullptr);
            return;
        }

        D_ASSERT(mCurrentHeapPtr != nullptr);
        mRetiredHeaps.push_back(mCurrentHeapPtr);
        mCurrentHeapPtr = nullptr;
        mCurrentOffset = 0;
    }

    void DynamicDescriptorHeap::RetireUsedHeaps(uint64_t fenceValue)
    {
        DiscardDescriptorHeaps(mDescriptorType, fenceValue, mRetiredHeaps);
        mRetiredHeaps.clear();
    }

    DynamicDescriptorHeap::DynamicDescriptorHeap(CommandContext& OwningContext, D3D12_DESCRIPTOR_HEAP_TYPE HeapType)
        : mOwningContext(OwningContext), mDescriptorType(HeapType)
    {
        mCurrentHeapPtr = nullptr;
        mCurrentOffset = 0;
        mDescriptorSize = D_RENDERER_DEVICE::GetDevice()->GetDescriptorHandleIncrementSize(HeapType);
    }

    DynamicDescriptorHeap::~DynamicDescriptorHeap()
    {
    }

    void DynamicDescriptorHeap::CleanupUsedHeaps(uint64_t fenceValue)
    {
        RetireCurrentHeap();
        RetireUsedHeaps(fenceValue);
        mGraphicsHandleCache.ClearCache();
        mComputeHandleCache.ClearCache();
    }

    inline ID3D12DescriptorHeap* DynamicDescriptorHeap::GetHeapPointer()
    {
        if (mCurrentHeapPtr == nullptr)
        {
            D_ASSERT(mCurrentOffset == 0);
            mCurrentHeapPtr = RequestDescriptorHeap(mDescriptorType);
            mFirstDescriptor = DescriptorHandle(
                mCurrentHeapPtr->GetCPUDescriptorHandleForHeapStart(),
                mCurrentHeapPtr->GetGPUDescriptorHandleForHeapStart());
        }

        return mCurrentHeapPtr;
    }

    uint32_t DynamicDescriptorHeap::DescriptorHandleCache::ComputeStagedSize()
    {
        // Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
        uint32_t NeededSpace = 0;
        uint32_t RootIndex;
        uint32_t StaleParams = mStaleRootParamsBitMap;
        while (_BitScanForward((unsigned long*)&RootIndex, StaleParams))
        {
            StaleParams ^= (1 << RootIndex);

            uint32_t MaxSetHandle;
            D_ASSERT_M(TRUE == _BitScanReverse((unsigned long*)&MaxSetHandle, m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
                "Root entry marked as stale but has no stale descriptors");

            NeededSpace += MaxSetHandle + 1;
        }
        return NeededSpace;
    }

    void DynamicDescriptorHeap::DescriptorHandleCache::CopyAndBindStaleTables(
        D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t DescriptorSize,
        DescriptorHandle DestHandleStart, ID3D12GraphicsCommandList* CmdList,
        void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
    {
        uint32_t StaleParamCount = 0;
        uint32_t TableSize[DescriptorHandleCache::kMaxNumDescriptorTables];
        uint32_t RootIndices[DescriptorHandleCache::kMaxNumDescriptorTables];
        uint32_t NeededSpace = 0;
        uint32_t RootIndex;

        // Sum the maximum assigned offsets of stale descriptor tables to determine total needed space.
        uint32_t StaleParams = mStaleRootParamsBitMap;
        while (_BitScanForward((unsigned long*)&RootIndex, StaleParams))
        {
            RootIndices[StaleParamCount] = RootIndex;
            StaleParams ^= (1 << RootIndex);

            uint32_t MaxSetHandle;
            D_ASSERT(TRUE == _BitScanReverse((unsigned long*)&MaxSetHandle, m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap),
                "Root entry marked as stale but has no stale descriptors");

            NeededSpace += MaxSetHandle + 1;
            TableSize[StaleParamCount] = MaxSetHandle + 1;

            ++StaleParamCount;
        }

        D_ASSERT(StaleParamCount <= DescriptorHandleCache::kMaxNumDescriptorTables,
            "We're only equipped to handle so many descriptor tables");

        mStaleRootParamsBitMap = 0;

        static const uint32_t kMaxDescriptorsPerCopy = 16;
        UINT NumDestDescriptorRanges = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE pDestDescriptorRangeStarts[kMaxDescriptorsPerCopy];
        UINT pDestDescriptorRangeSizes[kMaxDescriptorsPerCopy];

        UINT NumSrcDescriptorRanges = 0;
        D3D12_CPU_DESCRIPTOR_HANDLE pSrcDescriptorRangeStarts[kMaxDescriptorsPerCopy];
        UINT pSrcDescriptorRangeSizes[kMaxDescriptorsPerCopy];

        for (uint32_t i = 0; i < StaleParamCount; ++i)
        {
            RootIndex = RootIndices[i];
            (CmdList->*SetFunc)(RootIndex, DestHandleStart);

            DescriptorTableCache& RootDescTable = m_RootDescriptorTable[RootIndex];

            D3D12_CPU_DESCRIPTOR_HANDLE* SrcHandles = RootDescTable.TableStart;
            uint64_t SetHandles = (uint64_t)RootDescTable.AssignedHandlesBitMap;
            D3D12_CPU_DESCRIPTOR_HANDLE CurDest = DestHandleStart;
            DestHandleStart += TableSize[i] * DescriptorSize;

            unsigned long SkipCount;
            while (_BitScanForward64(&SkipCount, SetHandles))
            {
                // Skip over unset descriptor handles
                SetHandles >>= SkipCount;
                SrcHandles += SkipCount;
                CurDest.ptr += SkipCount * DescriptorSize;

                unsigned long DescriptorCount;
                _BitScanForward64(&DescriptorCount, ~SetHandles);
                SetHandles >>= DescriptorCount;

                // If we run out of temp room, copy what we've got so far
                if (NumSrcDescriptorRanges + DescriptorCount > kMaxDescriptorsPerCopy)
                {
                    D_RENDERER_DEVICE::GetDevice()->CopyDescriptors(
                        NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
                        NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
                        Type);

                    NumSrcDescriptorRanges = 0;
                    NumDestDescriptorRanges = 0;
                }

                // Setup destination range
                pDestDescriptorRangeStarts[NumDestDescriptorRanges] = CurDest;
                pDestDescriptorRangeSizes[NumDestDescriptorRanges] = DescriptorCount;
                ++NumDestDescriptorRanges;

                // Setup source ranges (one descriptor each because we don't assume they are contiguous)
                for (uint32_t j = 0; j < DescriptorCount; ++j)
                {
                    pSrcDescriptorRangeStarts[NumSrcDescriptorRanges] = SrcHandles[j];
                    pSrcDescriptorRangeSizes[NumSrcDescriptorRanges] = 1;
                    ++NumSrcDescriptorRanges;
                }

                // Move the destination pointer forward by the number of descriptors we will copy
                SrcHandles += DescriptorCount;
                CurDest.ptr += DescriptorCount * DescriptorSize;
            }
        }

        D_RENDERER_DEVICE::GetDevice()->CopyDescriptors(
            NumDestDescriptorRanges, pDestDescriptorRangeStarts, pDestDescriptorRangeSizes,
            NumSrcDescriptorRanges, pSrcDescriptorRangeStarts, pSrcDescriptorRangeSizes,
            Type);
    }

    void DynamicDescriptorHeap::CopyAndBindStagedTables(DescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList,
        void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE))
    {
        uint32_t NeededSize = HandleCache.ComputeStagedSize();
        if (!HasSpace(NeededSize))
        {
            RetireCurrentHeap();
            UnbindAllValid();
            NeededSize = HandleCache.ComputeStagedSize();
        }

        // This can trigger the creation of a new heap
        mOwningContext.SetDescriptorHeap(mDescriptorType, GetHeapPointer());
        HandleCache.CopyAndBindStaleTables(mDescriptorType, mDescriptorSize, Allocate(NeededSize), CmdList, SetFunc);
    }

    void DynamicDescriptorHeap::UnbindAllValid(void)
    {
        mGraphicsHandleCache.UnbindAllValid();
        mComputeHandleCache.UnbindAllValid();
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DynamicDescriptorHeap::UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE Handle)
    {
        if (!HasSpace(1))
        {
            RetireCurrentHeap();
            UnbindAllValid();
        }

        mOwningContext.SetDescriptorHeap(mDescriptorType, GetHeapPointer());

        DescriptorHandle DestHandle = mFirstDescriptor + mCurrentOffset * mDescriptorSize;
        mCurrentOffset += 1;

        D_RENDERER_DEVICE::GetDevice()->CopyDescriptorsSimple(1, DestHandle, Handle, mDescriptorType);

        return DestHandle;
    }

    void DynamicDescriptorHeap::DescriptorHandleCache::UnbindAllValid()
    {
        mStaleRootParamsBitMap = 0;

        unsigned long TableParams = mRootDescriptorTablesBitMap;
        unsigned long RootIndex;
        while (_BitScanForward(&RootIndex, TableParams))
        {
            TableParams ^= (1 << RootIndex);
            if (m_RootDescriptorTable[RootIndex].AssignedHandlesBitMap != 0)
                mStaleRootParamsBitMap |= (1 << RootIndex);
        }
    }

    void DynamicDescriptorHeap::DescriptorHandleCache::StageDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
    {
        D_ASSERT(((1 << RootIndex) & mRootDescriptorTablesBitMap) != 0, "Root parameter is not a CBV_SRV_UAV descriptor table");
        D_ASSERT(Offset + NumHandles <= m_RootDescriptorTable[RootIndex].TableSize);

        DescriptorTableCache& TableCache = m_RootDescriptorTable[RootIndex];
        D3D12_CPU_DESCRIPTOR_HANDLE* CopyDest = TableCache.TableStart + Offset;
        for (UINT i = 0; i < NumHandles; ++i)
            CopyDest[i] = Handles[i];
        TableCache.AssignedHandlesBitMap |= ((1 << NumHandles) - 1) << Offset;
        mStaleRootParamsBitMap |= (1 << RootIndex);
    }

    void DynamicDescriptorHeap::DescriptorHandleCache::ParseRootSignature(D3D12_DESCRIPTOR_HEAP_TYPE Type, const RootSignature& RootSig)
    {
        UINT CurrentOffset = 0;

        D_ASSERT(RootSig.mNumParameters <= 16, "Maybe we need to support something greater");

        mStaleRootParamsBitMap = 0;
        mRootDescriptorTablesBitMap = (Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ?
            RootSig.mSamplerTableBitMap : RootSig.mDescriptorTableBitMap);

        unsigned long TableParams = mRootDescriptorTablesBitMap;
        unsigned long RootIndex;
        while (_BitScanForward(&RootIndex, TableParams))
        {
            TableParams ^= (1 << RootIndex);

            UINT TableSize = RootSig.mDescriptorTableSize[RootIndex];
            D_ASSERT(TableSize > 0);

            DescriptorTableCache& RootDescriptorTable = m_RootDescriptorTable[RootIndex];
            RootDescriptorTable.AssignedHandlesBitMap = 0;
            RootDescriptorTable.TableStart = m_HandleCache + CurrentOffset;
            RootDescriptorTable.TableSize = TableSize;

            CurrentOffset += TableSize;
        }

        mMaxCachedDescriptors = CurrentOffset;

        D_ASSERT(mMaxCachedDescriptors <= kMaxNumDescriptors, "Exceeded user-supplied maximum cache size");
    }
}