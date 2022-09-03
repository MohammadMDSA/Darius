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
// Author(s):  James Stanard
//

#pragma once
#include "Renderer/GraphicsUtils/GpuResource.hpp"


#ifndef D_GRAPHICS_BUFFERS
#define D_GRAPHICS_BUFFERS Darius::Graphics::Utils::Buffers
#endif

namespace Darius::Graphics::Utils::Buffers
{
    class Texture : public GpuResource
    {
        friend class CommandContext;

    public:

        Texture() { m_hCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
        Texture(D3D12_CPU_DESCRIPTOR_HANDLE Handle) : m_hCpuDescriptorHandle(Handle) {}

        // Create a 1-level textures
        void Create2D(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitData);
        void CreateCube(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData);

        void CreateTGAFromMemory(const void* memBuffer, size_t fileSize, bool sRGB);
        bool CreateDDSFromMemory(const void* memBuffer, size_t fileSize, bool sRGB);
        void CreatePIXImageFromMemory(const void* memBuffer, size_t fileSize);

        virtual void Destroy() override
        {
            GpuResource::Destroy();
            m_hCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_hCpuDescriptorHandle; }

        uint32_t GetWidth() const { return mWidth; }
        uint32_t GetHeight() const { return mHeight; }
        uint32_t GetDepth() const { return mDepth; }

    protected:

        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mDepth;

        D3D12_CPU_DESCRIPTOR_HANDLE m_hCpuDescriptorHandle;
    };
}