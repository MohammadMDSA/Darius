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

#include "DepthBuffer.hpp"
#include "Renderer/CommandContext.hpp"

#ifndef D_GRAPHICS_BUFFERS
#define D_GRAPHICS_BUFFERS Darius::Graphics::Utils::Buffers
#endif

namespace Darius::Graphics::Utils::Buffers
{
    class ShadowBuffer : public DepthBuffer
    {
    public:
        ShadowBuffer() {}

        void Create(const std::wstring& Name, uint32_t Width, uint32_t Height,
            D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
        void Create(const std::wstring& Name, uint32_t Width, uint32_t Height);

        D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return GetDepthSRV(); }

        void BeginRendering(GraphicsContext& context);
        void EndRendering(GraphicsContext& context);

    private:
        D3D12_VIEWPORT mViewport;
        D3D12_RECT mScissor;
    };
}