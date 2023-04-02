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

#include <vector>
#include <queue>
#include <mutex>
#include <stdint.h>
#include "CommandAllocatorPool.hpp"


#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics
{
    class CommandContext;
}

namespace Darius::Graphics::Utils
{
    class CommandQueue
    {
        friend class CommandListManager;
        friend class Darius::Graphics::CommandContext;

    public:
        CommandQueue(D3D12_COMMAND_LIST_TYPE Type);
        ~CommandQueue();

        void Create(ID3D12Device* pDevice);
        void Shutdown();

        inline bool IsReady()
        {
            return mCommandQueue != nullptr;
        }

        uint64_t IncrementFence(void);
        bool IsFenceComplete(uint64_t FenceValue);
        void StallForFence(uint64_t FenceValue);
        void StallForProducer(CommandQueue& Producer);
        void WaitForFence(uint64_t FenceValue);
        void WaitForIdle(void) { WaitForFence(IncrementFence()); }

        ID3D12CommandQueue* GetCommandQueue() { return mCommandQueue; }

        uint64_t GetNextFenceValue() { return mNextFenceValue; }

    private:

        uint64_t ExecuteCommandList(ID3D12CommandList* List);
        ID3D12CommandAllocator* RequestAllocator(void);
        void DiscardAllocator(uint64_t FenceValueForReset, ID3D12CommandAllocator* Allocator);

        ID3D12CommandQueue* mCommandQueue;

        const D3D12_COMMAND_LIST_TYPE mType;

        CommandAllocatorPool mAllocatorPool;
        std::mutex mFenceMutex;
        std::mutex mEventMutex;

        // Lifetime of these objects is managed by the descriptor cache
        ID3D12Fence* mFence;
        uint64_t mNextFenceValue;
        uint64_t mLastCompletedFenceValue;
        HANDLE mFenceEventHandle;

    };

    class CommandListManager
    {
        friend class CommandContext;

    public:
        CommandListManager();
        ~CommandListManager();

        void Create(ID3D12Device* pDevice);
        void Shutdown();

        CommandQueue& GetGraphicsQueue(void) { return mGraphicsQueue; }
        CommandQueue& GetComputeQueue(void) { return mComputeQueue; }
        CommandQueue& GetCopyQueue(void) { return mCopyQueue; }

        CommandQueue& GetQueue(D3D12_COMMAND_LIST_TYPE Type = D3D12_COMMAND_LIST_TYPE_DIRECT)
        {
            switch (Type)
            {
            case D3D12_COMMAND_LIST_TYPE_COMPUTE: return mComputeQueue;
            case D3D12_COMMAND_LIST_TYPE_COPY: return mCopyQueue;
            default: return mGraphicsQueue;
            }
        }

        ID3D12CommandQueue* GetCommandQueue()
        {
            return mGraphicsQueue.GetCommandQueue();
        }

        void CreateNewCommandList(
            D3D12_COMMAND_LIST_TYPE Type,
            ID3D12GraphicsCommandList** List,
            ID3D12CommandAllocator** Allocator);

        // Test to see if a fence has already been reached
        bool IsFenceComplete(uint64_t FenceValue)
        {
            return GetQueue(D3D12_COMMAND_LIST_TYPE(FenceValue >> 56)).IsFenceComplete(FenceValue);
        }

        // The CPU will wait for a fence to reach a specified value
        void WaitForFence(uint64_t FenceValue);

        // The CPU will wait for all command queues to empty (so that the GPU is idle)
        void IdleGPU(void)
        {
            mGraphicsQueue.WaitForIdle();
            mComputeQueue.WaitForIdle();
            mCopyQueue.WaitForIdle();
        }

    private:

        ID3D12Device* mDevice;

        CommandQueue mGraphicsQueue;
        CommandQueue mComputeQueue;
        CommandQueue mCopyQueue;
    };
}