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
#include "CommandListManager.hpp"
#include "Renderer/GraphicsCore.hpp"

#include <Utils/Assert.hpp>

namespace Darius::Graphics::Utils
{
    CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE Type) :
        mType(Type),
        mCommandQueue(nullptr),
        mFence(nullptr),
        mNextFenceValue((uint64_t)Type << 56 | 1),
        mLastCompletedFenceValue((uint64_t)Type << 56),
        mAllocatorPool(Type)
    {
    }

    CommandQueue::~CommandQueue()
    {
        Shutdown();
    }

    void CommandQueue::Shutdown()
    {
        if (mCommandQueue == nullptr)
            return;

        mAllocatorPool.Shutdown();

        CloseHandle(mFenceEventHandle);

        mFence->Release();
        mFence = nullptr;

        mCommandQueue->Release();
        mCommandQueue = nullptr;
    }

    CommandListManager::CommandListManager() :
        mDevice(nullptr),
        mGraphicsQueue(D3D12_COMMAND_LIST_TYPE_DIRECT),
        mComputeQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE),
        mCopyQueue(D3D12_COMMAND_LIST_TYPE_COPY)
    {
    }

    CommandListManager::~CommandListManager()
    {
        Shutdown();
    }

    void CommandListManager::Shutdown()
    {
        mGraphicsQueue.Shutdown();
        mComputeQueue.Shutdown();
        mCopyQueue.Shutdown();
    }

    void CommandQueue::Create(ID3D12Device* pDevice)
    {
        D_ASSERT(pDevice != nullptr);
        D_ASSERT(!IsReady());
        D_ASSERT(mAllocatorPool.Size() == 0);

        D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
        QueueDesc.Type = mType;
        QueueDesc.NodeMask = 1;
        pDevice->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&mCommandQueue));
        mCommandQueue->SetName(L"CommandListManager::m_CommandQueue");

        D_HR_CHECK(pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));
        mFence->SetName(L"CommandListManager::m_pFence");
        mFence->Signal((uint64_t)mType << 56);

        mFenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
        D_ASSERT(mFenceEventHandle != NULL);

        mAllocatorPool.Create(pDevice);

        D_ASSERT(IsReady());
    }

    void CommandListManager::Create(ID3D12Device* pDevice)
    {
        D_ASSERT(pDevice != nullptr);

        mDevice = pDevice;

        mGraphicsQueue.Create(pDevice);
        mComputeQueue.Create(pDevice);
        mCopyQueue.Create(pDevice);
    }

    void CommandListManager::CreateNewCommandList(D3D12_COMMAND_LIST_TYPE Type, ID3D12GraphicsCommandList** List, ID3D12CommandAllocator** Allocator)
    {
        D_ASSERT(Type != D3D12_COMMAND_LIST_TYPE_BUNDLE, "Bundles are not yet supported");
        switch (Type)
        {
        case D3D12_COMMAND_LIST_TYPE_DIRECT: *Allocator = mGraphicsQueue.RequestAllocator(); break;
        case D3D12_COMMAND_LIST_TYPE_BUNDLE: break;
        case D3D12_COMMAND_LIST_TYPE_COMPUTE: *Allocator = mComputeQueue.RequestAllocator(); break;
        case D3D12_COMMAND_LIST_TYPE_COPY: *Allocator = mCopyQueue.RequestAllocator(); break;
        }

        D_HR_CHECK(mDevice->CreateCommandList(1, Type, *Allocator, nullptr, IID_PPV_ARGS(List)));
        (*List)->SetName(L"CommandList");
    }

    uint64_t CommandQueue::ExecuteCommandList(ID3D12CommandList* List)
    {
        std::lock_guard<std::mutex> LockGuard(mFenceMutex);

        D_HR_CHECK(((ID3D12GraphicsCommandList*)List)->Close());

        // Kickoff the command list
        mCommandQueue->ExecuteCommandLists(1, &List);

        // Signal the next fence value (with the GPU)
        mCommandQueue->Signal(mFence, mNextFenceValue);

        // And increment the fence value.  
        return mNextFenceValue++;
    }

    uint64_t CommandQueue::IncrementFence(void)
    {
        std::lock_guard<std::mutex> LockGuard(mFenceMutex);
        mCommandQueue->Signal(mFence, mNextFenceValue);
        return mNextFenceValue++;
    }

    bool CommandQueue::IsFenceComplete(uint64_t FenceValue)
    {
        // Avoid querying the fence value by testing against the last one seen.
        // The max() is to protect against an unlikely race condition that could cause the last
        // completed fence value to regress.
        if (FenceValue > mLastCompletedFenceValue)
            mLastCompletedFenceValue = std::max(mLastCompletedFenceValue, mFence->GetCompletedValue());

        return FenceValue <= mLastCompletedFenceValue;
    }

    void CommandQueue::StallForFence(uint64_t FenceValue)
    {
        CommandQueue& Producer = D_GRAPHICS::GetCommandManager()->GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
        mCommandQueue->Wait(Producer.mFence, FenceValue);
    }

    void CommandQueue::StallForProducer(CommandQueue& Producer)
    {
        D_ASSERT(Producer.mNextFenceValue > 0);
        mCommandQueue->Wait(Producer.mFence, Producer.mNextFenceValue - 1);
    }

    void CommandQueue::WaitForFence(uint64_t FenceValue)
    {
        if (IsFenceComplete(FenceValue))
            return;

        // TODO:  Think about how this might affect a multi-threaded situation.  Suppose thread A
        // wants to wait for fence 100, then thread B comes along and wants to wait for 99.  If
        // the fence can only have one event set on completion, then thread B has to wait for 
        // 100 before it knows 99 is ready.  Maybe insert sequential events?
        {
            std::lock_guard<std::mutex> LockGuard(mEventMutex);

            mFence->SetEventOnCompletion(FenceValue, mFenceEventHandle);
            WaitForSingleObject(mFenceEventHandle, INFINITE);
            mLastCompletedFenceValue = FenceValue;
        }
    }

    void CommandListManager::WaitForFence(uint64_t FenceValue)
    {
        CommandQueue& Producer = D_GRAPHICS::GetCommandManager()->GetQueue((D3D12_COMMAND_LIST_TYPE)(FenceValue >> 56));
        Producer.WaitForFence(FenceValue);
    }

    ID3D12CommandAllocator* CommandQueue::RequestAllocator()
    {
        uint64_t CompletedFence = mFence->GetCompletedValue();

        return mAllocatorPool.RequestAllocator(CompletedFence);
    }

    void CommandQueue::DiscardAllocator(uint64_t FenceValue, ID3D12CommandAllocator* Allocator)
    {
        mAllocatorPool.DiscardAllocator(FenceValue, Allocator);
    }
}