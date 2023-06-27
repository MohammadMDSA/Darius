#include "Graphics/pch.hpp"
#include "DepthBuffer.hpp"
#include "Graphics/GraphicsCore.hpp"
#include "Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp"
#include "Graphics/GraphicsDeviceManager.hpp"

using namespace DirectX;

namespace Darius::Graphics::Utils::Buffers
{
    void DepthBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        Create(Name, Width, Height, 1, Format, VidMemPtr);
    }

    void DepthBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t Samples, DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr)
    {
        Width = XMMax(1u, Width);
        Height = XMMax(1u, Height);

        D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(Width, Height, 1, 1, Format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
        ResourceDesc.SampleDesc.Count = Samples;

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.Format = Format;
        ClearValue.DepthStencil.Depth = mClearDepth;
        ClearValue.DepthStencil.Stencil = mClearStencil;
        CreateTextureResource(D_GRAPHICS_DEVICE::GetDevice(), Name, ResourceDesc, ClearValue, VidMemPtr, D3D12_RESOURCE_STATE_DEPTH_WRITE);
        CreateDerivedViews(D_GRAPHICS_DEVICE::GetDevice(), Format);
    }

    void DepthBuffer::CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format)
    {
        ID3D12Resource* Resource = mResource.Get();

        D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsvDesc.Format = GetDSVFormat(Format);
        if (Resource->GetDesc().SampleDesc.Count == 1)
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = 0;
        }
        else
        {
            dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
        }

        if (mDSV[0].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        {
            mDSV[0] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            mDSV[1] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        }

        dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
        Device->CreateDepthStencilView(Resource, &dsvDesc, mDSV[0]);

        dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
        Device->CreateDepthStencilView(Resource, &dsvDesc, mDSV[1]);

        DXGI_FORMAT stencilReadFormat = GetStencilFormat(Format);
        if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
        {
            if (mDSV[2].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
            {
                mDSV[2] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
                mDSV[3] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
            }

            dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
            Device->CreateDepthStencilView(Resource, &dsvDesc, mDSV[2]);

            dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
            Device->CreateDepthStencilView(Resource, &dsvDesc, mDSV[3]);
        }
        else
        {
            mDSV[2] = mDSV[0];
            mDSV[3] = mDSV[1];
        }

        if (mDepthSRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
            mDepthSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        // Create the shader resource view
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
        SRVDesc.Format = GetDepthFormat(Format);
        if (dsvDesc.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2D)
        {
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = 1;
        }
        else
        {
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        Device->CreateShaderResourceView(Resource, &SRVDesc, mDepthSRV);

        if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
        {
            if (mStencilSRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
                mStencilSRV = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            SRVDesc.Format = stencilReadFormat;
            Device->CreateShaderResourceView(Resource, &SRVDesc, mStencilSRV);
        }
    }
}