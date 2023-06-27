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

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics::Utils
{
    class CommandAllocatorPool
    {
    public:
        CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type);
        ~CommandAllocatorPool();

        void Create(ID3D12Device* pDevice);
        void Shutdown();

        ID3D12CommandAllocator* RequestAllocator(uint64_t CompletedFenceValue);
        void DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator);

        inline size_t Size() { return mAllocatorPool.size(); }

    private:
        const D3D12_COMMAND_LIST_TYPE mCommandListType;

        ID3D12Device* mDevice;
        std::vector<ID3D12CommandAllocator*> mAllocatorPool;
        std::queue<std::pair<uint64_t, ID3D12CommandAllocator*>> mReadyAllocators;
        std::mutex mAllocatorMutex;
    };
}