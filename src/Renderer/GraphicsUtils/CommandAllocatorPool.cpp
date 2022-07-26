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
#include "CommandAllocatorPool.hpp"

namespace Darius::Graphics::Utils
{
    CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE Type) :
        mCommandListType(Type),
        mDevice(nullptr)
    {
    }

    CommandAllocatorPool::~CommandAllocatorPool()
    {
        Shutdown();
    }

    void CommandAllocatorPool::Create(ID3D12Device* pDevice)
    {
        mDevice = pDevice;
    }

    void CommandAllocatorPool::Shutdown()
    {
        for (size_t i = 0; i < mAllocatorPool.size(); ++i)
            mAllocatorPool[i]->Release();

        mAllocatorPool.clear();
    }

    ID3D12CommandAllocator* CommandAllocatorPool::RequestAllocator(uint64_t CompletedFenceValue)
    {
        std::lock_guard<std::mutex> LockGuard(mAllocatorMutex);

        ID3D12CommandAllocator* pAllocator = nullptr;

        if (!mReadyAllocators.empty())
        {
            std::pair<uint64_t, ID3D12CommandAllocator*>& AllocatorPair = mReadyAllocators.front();

            if (AllocatorPair.first <= CompletedFenceValue)
            {
                pAllocator = AllocatorPair.second;
                D_HR_CHECK(pAllocator->Reset());
                mReadyAllocators.pop();
            }
        }

        // If no allocator's were ready to be reused, create a new one
        if (pAllocator == nullptr)
        {
            D_HR_CHECK(mDevice->CreateCommandAllocator(mCommandListType, IID_PPV_ARGS(&pAllocator)));
            wchar_t AllocatorName[32];
            swprintf(AllocatorName, 32, L"CommandAllocator %zu", mAllocatorPool.size());
            pAllocator->SetName(AllocatorName);
            mAllocatorPool.push_back(pAllocator);
        }

        return pAllocator;
    }

    void CommandAllocatorPool::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator)
    {
        std::lock_guard<std::mutex> LockGuard(mAllocatorMutex);

        // That fence value indicates we are free to reset the allocator
        mReadyAllocators.push(std::make_pair(FenceValue, Allocator));
    }
}