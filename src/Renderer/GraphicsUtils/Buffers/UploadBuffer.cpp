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
#include "UploadBuffer.hpp"
#include "Renderer/GraphicsCore.hpp"
#include "Renderer/GraphicsDeviceManager.hpp"

namespace Darius::Graphics::Utils::Buffers
{
    void UploadBuffer::Create(const std::wstring& name, size_t BufferSize)
    {
        Destroy();

        mBufferSize = BufferSize;

        // Create an upload buffer.  This is CPU-visible, but it's write combined memory, so
        // avoid reading back from it.
        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;

        // Upload buffers must be 1-dimensional
        D3D12_RESOURCE_DESC ResourceDesc = {};
        ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        ResourceDesc.Width = mBufferSize;
        ResourceDesc.Height = 1;
        ResourceDesc.DepthOrArraySize = 1;
        ResourceDesc.MipLevels = 1;
        ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        ResourceDesc.SampleDesc.Count = 1;
        ResourceDesc.SampleDesc.Quality = 0;
        ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D_HR_CHECK(D_GRAPHICS_DEVICE::GetDevice()->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&mResource)));

        mGpuVirtualAddress = mResource->GetGPUVirtualAddress();

#ifdef RELEASE
        (name);
#else
        mResource->SetName(name.c_str());
#endif
    }


    void* UploadBuffer::Map(void)
    {
        void* Memory;
        auto range = CD3DX12_RANGE(0, mBufferSize);
        mResource->Map(0, &range, &Memory);
        return Memory;
    }

    void UploadBuffer::Unmap(size_t begin, size_t end)
    {
        auto range = CD3DX12_RANGE(begin, std::min(end, mBufferSize));
        mResource->Unmap(0, &range);
    }
}