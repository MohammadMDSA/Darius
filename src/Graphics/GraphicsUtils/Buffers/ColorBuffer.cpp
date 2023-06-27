#include "Graphics/pch.hpp"
#include "ColorBuffer.hpp"

#include "Graphics/GraphicsCore.hpp"
#include "Graphics/CommandContext.hpp"
#include "Graphics/GraphicsDeviceManager.hpp"

using namespace D_GRAPHICS;
using namespace DirectX;

namespace Darius::Graphics::Utils::Buffers
{
    void ColorBuffer::CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format, uint32_t ArraySize, uint32_t NumMips, bool cube)
    {
        D_ASSERT_M(ArraySize == 1 || NumMips == 1, "We don't support auto-mips on texture arrays");

        mNumMipMaps = NumMips - 1;

        D3D12_RENDER_TARGET_VIEW_DESC RTVDesc = {};
        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
        D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};

        RTVDesc.Format = Format;
        UAVDesc.Format = GetUAVFormat(Format);
        SRVDesc.Format = Format;
        SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        if (ArraySize > 1 && cube)
        {
            D_ASSERT_M(ArraySize % 6 == 0, "The array size should be a multiple of 6");
            RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            RTVDesc.Texture2DArray.MipSlice = 0;
            RTVDesc.Texture2DArray.FirstArraySlice = 0;
            RTVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

            UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            UAVDesc.Texture2DArray.MipSlice = 0;
            UAVDesc.Texture2DArray.FirstArraySlice = 0;
            UAVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
            SRVDesc.TextureCubeArray.MipLevels = NumMips;
            SRVDesc.TextureCubeArray.MostDetailedMip = 0;
            SRVDesc.TextureCubeArray.First2DArrayFace = 0;
            SRVDesc.TextureCubeArray.NumCubes = (UINT)ArraySize / 6u;
            SRVDesc.TextureCubeArray.ResourceMinLODClamp = 0.f;
        }
        else if (ArraySize > 1)
        {
            RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
            RTVDesc.Texture2DArray.MipSlice = 0;
            RTVDesc.Texture2DArray.FirstArraySlice = 0;
            RTVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

            UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
            UAVDesc.Texture2DArray.MipSlice = 0;
            UAVDesc.Texture2DArray.FirstArraySlice = 0;
            UAVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;

            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            SRVDesc.Texture2DArray.MipLevels = NumMips;
            SRVDesc.Texture2DArray.MostDetailedMip = 0;
            SRVDesc.Texture2DArray.FirstArraySlice = 0;
            SRVDesc.Texture2DArray.ArraySize = (UINT)ArraySize;
        }
        else if (mFragmentCount > 1)
        {
            RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
        }
        else
        {
            RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            RTVDesc.Texture2D.MipSlice = 0;

            UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
            UAVDesc.Texture2D.MipSlice = 0;

            SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            SRVDesc.Texture2D.MipLevels = NumMips;
            SRVDesc.Texture2D.MostDetailedMip = 0;
        }

        if (mSrvHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
        {
            mRtvHandle = D_GRAPHICS::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
            mSrvHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        }

        ID3D12Resource* Resource = mResource.Get();

        // Create the render target view
        Device->CreateRenderTargetView(Resource, &RTVDesc, mRtvHandle);

        // Create the shader resource view
        Device->CreateShaderResourceView(Resource, &SRVDesc, mSrvHandle);

        if (mFragmentCount > 1)
            return;

        // Create the UAVs for each mip level (RWTexture2D)
        for (uint32_t i = 0; i < NumMips; ++i)
        {
            if (mUavHandle[i].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
                mUavHandle[i] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

            Device->CreateUnorderedAccessView(Resource, nullptr, &UAVDesc, mUavHandle[i]);

            UAVDesc.Texture2D.MipSlice++;
        }
    }

    void ColorBuffer::CreateFromSwapChain(const std::wstring& Name, ID3D12Resource* BaseResource)
    {
        AssociateWithResource(D_GRAPHICS_DEVICE::GetDevice(), Name, BaseResource, D3D12_RESOURCE_STATE_PRESENT);

        //m_UAVHandle[0] = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        //Graphics::g_Device->CreateUnorderedAccessView(m_pResource.Get(), nullptr, nullptr, m_UAVHandle[0]);

        mRtvHandle = Graphics::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D_GRAPHICS_DEVICE::GetDevice()->CreateRenderTargetView(mResource.Get(), nullptr, mRtvHandle);
    }

    void ColorBuffer::Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumMips,
        DXGI_FORMAT Format, D3D12_GPU_VIRTUAL_ADDRESS VidMem)
    {
        Width = XMMax(1u, Width);
        Height = XMMax(1u, Height);
        NumMips = (NumMips == 0 ? ComputeNumMips(Width, Height) : NumMips);
        D3D12_RESOURCE_FLAGS Flags = CombineResourceFlags();
        D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(Width, Height, 1, NumMips, Format, Flags);

        ResourceDesc.SampleDesc.Count = mFragmentCount;
        ResourceDesc.SampleDesc.Quality = 0;

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.Format = Format;
        ClearValue.Color[0] = mClearColor.GetR();
        ClearValue.Color[1] = mClearColor.GetG();
        ClearValue.Color[2] = mClearColor.GetB();
        ClearValue.Color[3] = mClearColor.GetA();

        CreateTextureResource(D_GRAPHICS_DEVICE::GetDevice(), Name, ResourceDesc, ClearValue, VidMem);
        CreateDerivedViews(D_GRAPHICS_DEVICE::GetDevice(), Format, 1, NumMips);
    }

    void ColorBuffer::CreateArray(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t ArrayCount,
        DXGI_FORMAT Format, bool cube, D3D12_GPU_VIRTUAL_ADDRESS VidMem)
    {
        D3D12_RESOURCE_FLAGS Flags = CombineResourceFlags();
        D3D12_RESOURCE_DESC ResourceDesc = DescribeTex2D(Width, Height, ArrayCount, 1, Format, Flags);

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.Format = Format;
        ClearValue.Color[0] = mClearColor.GetR();
        ClearValue.Color[1] = mClearColor.GetG();
        ClearValue.Color[2] = mClearColor.GetB();
        ClearValue.Color[3] = mClearColor.GetA();

        CreateTextureResource(D_GRAPHICS_DEVICE::GetDevice(), Name, ResourceDesc, ClearValue, VidMem);
        CreateDerivedViews(D_GRAPHICS_DEVICE::GetDevice(), Format, ArrayCount, 1, cube);
    }

    void ColorBuffer::GenerateMipMaps(CommandContext& BaseContext)
    {
        if (mNumMipMaps == 0)
            return;

        ComputeContext& Context = BaseContext.GetComputeContext();

        Context.SetRootSignature(D_GRAPHICS::CommonRS);

        Context.TransitionResource(*this, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
        Context.SetDynamicDescriptor(1, 0, mSrvHandle);

        for (uint32_t TopMip = 0; TopMip < mNumMipMaps; )
        {
            uint32_t SrcWidth = mWidth >> TopMip;
            uint32_t SrcHeight = mHeight >> TopMip;
            uint32_t DstWidth = SrcWidth >> 1;
            uint32_t DstHeight = SrcHeight >> 1;

            // Determine if the first downsample is more than 2:1.  This happens whenever
            // the source width or height is odd.
            uint32_t NonPowerOfTwo = (SrcWidth & 1) | (SrcHeight & 1) << 1;
            if (mFormat == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
                Context.SetPipelineState(D_GRAPHICS::GenerateMipsGammaPSO[NonPowerOfTwo]);
            else
                Context.SetPipelineState(D_GRAPHICS::GenerateMipsLinearPSO[NonPowerOfTwo]);

            // We can downsample up to four times, but if the ratio between levels is not
            // exactly 2:1, we have to shift our blend weights, which gets complicated or
            // expensive.  Maybe we can update the code later to compute sample weights for
            // each successive downsample.  We use _BitScanForward to count number of zeros
            // in the low bits.  Zeros indicate we can divide by two without truncating.
            uint32_t AdditionalMips;
            _BitScanForward((unsigned long*)&AdditionalMips,
                (DstWidth == 1 ? DstHeight : DstWidth) | (DstHeight == 1 ? DstWidth : DstHeight));
            uint32_t NumMips = 1 + (AdditionalMips > 3 ? 3 : AdditionalMips);
            if (TopMip + NumMips > mNumMipMaps)
                NumMips = mNumMipMaps - TopMip;

            // These are clamped to 1 after computing additional mips because clamped
            // dimensions should not limit us from downsampling multiple times.  (E.g.
            // 16x1 -> 8x1 -> 4x1 -> 2x1 -> 1x1.)
            if (DstWidth == 0)
                DstWidth = 1;
            if (DstHeight == 0)
                DstHeight = 1;

            Context.SetConstants(0, TopMip, NumMips, 1.0f / DstWidth, 1.0f / DstHeight);
            Context.SetDynamicDescriptors(2, 0, NumMips, mUavHandle + TopMip + 1);
            Context.Dispatch2D(DstWidth, DstHeight);

            Context.InsertUAVBarrier(*this);

            TopMip += NumMips;
        }

        Context.TransitionResource(*this, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }
}