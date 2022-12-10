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
#include "ShadowBuffer.hpp"

namespace Darius::Graphics::Utils::Buffers
{
    void ShadowBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        DepthBuffer::Create(Name, Width, Height, DXGI_FORMAT_D16_UNORM, VidMemPtr);

        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = (float)Width;
        mViewport.Height = (float)Height;
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;

        // Prevent drawing to the boundary pixels so that we don't have to worry about shadows stretching
        mScissor.left = 1;
        mScissor.top = 1;
        mScissor.right = (LONG)Width - 2;
        mScissor.bottom = (LONG)Height - 2;
    }

    void ShadowBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height)
    {
        DepthBuffer::Create(Name, Width, Height, DXGI_FORMAT_D16_UNORM);

        mViewport.TopLeftX = 0.0f;
        mViewport.TopLeftY = 0.0f;
        mViewport.Width = (float)Width;
        mViewport.Height = (float)Height;
        mViewport.MinDepth = 0.0f;
        mViewport.MaxDepth = 1.0f;

        // Prevent drawing to the boundary pixels so that we don't have to worry about shadows stretching
        mScissor.left = 1;
        mScissor.top = 1;
        mScissor.right = (LONG)Width - 2;
        mScissor.bottom = (LONG)Height - 2;
    }

    void ShadowBuffer::BeginRendering(GraphicsContext& Context)
    {
        Context.TransitionResource(*this, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
        Context.ClearDepth(*this);
        Context.SetDepthStencilTarget(GetDSV());
        Context.SetViewportAndScissor(mViewport, mScissor);
    }

    void ShadowBuffer::EndRendering(GraphicsContext& Context)
    {
        Context.TransitionResource(*this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

}