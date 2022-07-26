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
// Description:  An upload buffer is visible to both the CPU and the GPU, but
// because the memory is write combined, you should avoid reading data with the CPU.
// An upload buffer is intended for moving data to a default GPU buffer.  You can
// read from a file directly into an upload buffer, rather than reading into regular
// cached memory, copying that to an upload buffer, and copying that to the GPU.

#pragma once

#include "Renderer/GraphicsUtils/GpuResource.hpp"

#ifndef D_GRAPHICS_BUFFERS
#define D_GRAPHICS_BUFFERS Darius::Graphics::Utils::Buffers
#endif

namespace Darius::Graphics::Utils::Buffers
{
    class UploadBuffer : public GpuResource
    {
    public:
        virtual ~UploadBuffer() { Destroy(); }

        void Create(const std::wstring& name, size_t BufferSize);

        void* Map(void);
        void Unmap(size_t begin = 0, size_t end = -1);

        size_t GetBufferSize() const { return mBufferSize; }

    protected:

        size_t mBufferSize;
    };
}