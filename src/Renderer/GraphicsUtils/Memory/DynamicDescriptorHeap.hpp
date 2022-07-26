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

#pragma once

#include "DescriptorHeap.hpp"
#include "Renderer/GraphicsUtils/RootSignature.hpp"

#include <vector>
#include <queue>

#ifndef D_GRAPHICS_MEMORY
#define D_GRAPHICS_MEMORY Darius::Graphics::Utils::Memory
#endif

namespace Darius::Graphics
{
    class CommandContext;
}

using namespace D_GRAPHICS_UTILS;
using namespace Darius::Graphics;

namespace Darius::Graphics::Utils::Memory
{
    // This class is a linear allocation system for dynamically generated descriptor tables.  It internally caches
    // CPU descriptor handles so that when not enough space is available in the current heap, necessary descriptors
    // can be re-copied to the new heap.
    class DynamicDescriptorHeap
    {
    public:
        DynamicDescriptorHeap(CommandContext& OwningContext, D3D12_DESCRIPTOR_HEAP_TYPE HeapType);
        ~DynamicDescriptorHeap();

        static void DestroyAll(void)
        {
            sm_DescriptorHeapPool[0].clear();
            sm_DescriptorHeapPool[1].clear();
        }

        void CleanupUsedHeaps(uint64_t fenceValue);

        // Copy multiple handles into the cache area reserved for the specified root parameter.
        void SetGraphicsDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
        {
            mGraphicsHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
        }

        void SetComputeDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
        {
            mComputeHandleCache.StageDescriptorHandles(RootIndex, Offset, NumHandles, Handles);
        }

        // Bypass the cache and upload directly to the shader-visible heap
        D3D12_GPU_DESCRIPTOR_HANDLE UploadDirect(D3D12_CPU_DESCRIPTOR_HANDLE Handles);

        // Deduce cache layout needed to support the descriptor tables needed by the root signature.
        void ParseGraphicsRootSignature(const RootSignature& RootSig)
        {
            mGraphicsHandleCache.ParseRootSignature(mDescriptorType, RootSig);
        }

        void ParseComputeRootSignature(const RootSignature& RootSig)
        {
            mComputeHandleCache.ParseRootSignature(mDescriptorType, RootSig);
        }

        // Upload any new descriptors in the cache to the shader-visible heap.
        inline void CommitGraphicsRootDescriptorTables(ID3D12GraphicsCommandList* CmdList)
        {
            if (mGraphicsHandleCache.mStaleRootParamsBitMap != 0)
                CopyAndBindStagedTables(mGraphicsHandleCache, CmdList, &ID3D12GraphicsCommandList::SetGraphicsRootDescriptorTable);
        }

        inline void CommitComputeRootDescriptorTables(ID3D12GraphicsCommandList* CmdList)
        {
            if (mComputeHandleCache.mStaleRootParamsBitMap != 0)
                CopyAndBindStagedTables(mComputeHandleCache, CmdList, &ID3D12GraphicsCommandList::SetComputeRootDescriptorTable);
        }

    private:

        // Static members
        static const uint32_t kNumDescriptorsPerHeap = 1024;
        static std::mutex sm_Mutex;
        static std::vector<Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>> sm_DescriptorHeapPool[2];
        static std::queue<std::pair<uint64_t, ID3D12DescriptorHeap*>> sm_RetiredDescriptorHeaps[2];
        static std::queue<ID3D12DescriptorHeap*> sm_AvailableDescriptorHeaps[2];

        // Static methods
        static ID3D12DescriptorHeap* RequestDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE HeapType);
        static void DiscardDescriptorHeaps(D3D12_DESCRIPTOR_HEAP_TYPE HeapType, uint64_t FenceValueForReset, const std::vector<ID3D12DescriptorHeap*>& UsedHeaps);

        // Non-static members
        CommandContext& mOwningContext;
        ID3D12DescriptorHeap* mCurrentHeapPtr;
        const D3D12_DESCRIPTOR_HEAP_TYPE mDescriptorType;
        uint32_t mDescriptorSize;
        uint32_t mCurrentOffset;
        DescriptorHandle mFirstDescriptor;
        std::vector<ID3D12DescriptorHeap*> mRetiredHeaps;

        // Describes a descriptor table entry:  a region of the handle cache and which handles have been set
        struct DescriptorTableCache
        {
            DescriptorTableCache() : AssignedHandlesBitMap(0) {}
            uint32_t AssignedHandlesBitMap;
            D3D12_CPU_DESCRIPTOR_HANDLE* TableStart;
            uint32_t TableSize;
        };

        struct DescriptorHandleCache
        {
            DescriptorHandleCache()
            {
                ClearCache();
            }

            void ClearCache()
            {
                mRootDescriptorTablesBitMap = 0;
                mStaleRootParamsBitMap = 0;
                mMaxCachedDescriptors = 0;
            }

            uint32_t mRootDescriptorTablesBitMap;
            uint32_t mStaleRootParamsBitMap;
            uint32_t mMaxCachedDescriptors;

            static const uint32_t kMaxNumDescriptors = 256;
            static const uint32_t kMaxNumDescriptorTables = 16;

            uint32_t ComputeStagedSize();
            void CopyAndBindStaleTables(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32_t DescriptorSize, DescriptorHandle DestHandleStart, ID3D12GraphicsCommandList* CmdList,
                void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

            DescriptorTableCache m_RootDescriptorTable[kMaxNumDescriptorTables];
            D3D12_CPU_DESCRIPTOR_HANDLE m_HandleCache[kMaxNumDescriptors];

            void UnbindAllValid();
            void StageDescriptorHandles(UINT RootIndex, UINT Offset, UINT NumHandles, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
            void ParseRootSignature(D3D12_DESCRIPTOR_HEAP_TYPE Type, const RootSignature& RootSig);
        };

        DescriptorHandleCache mGraphicsHandleCache;
        DescriptorHandleCache mComputeHandleCache;

        bool HasSpace(uint32_t Count)
        {
            return (mCurrentHeapPtr != nullptr && mCurrentOffset + Count <= kNumDescriptorsPerHeap);
        }

        void RetireCurrentHeap(void);
        void RetireUsedHeaps(uint64_t fenceValue);
        ID3D12DescriptorHeap* GetHeapPointer();

        DescriptorHandle Allocate(UINT Count)
        {
            DescriptorHandle ret = mFirstDescriptor + mCurrentOffset * mDescriptorSize;
            mCurrentOffset += Count;
            return ret;
        }

        void CopyAndBindStagedTables(DescriptorHandleCache& HandleCache, ID3D12GraphicsCommandList* CmdList,
            void (STDMETHODCALLTYPE ID3D12GraphicsCommandList::* SetFunc)(UINT, D3D12_GPU_DESCRIPTOR_HANDLE));

        // Mark all descriptors in the cache as stale and in need of re-uploading.
        void UnbindAllValid(void);

    };
}