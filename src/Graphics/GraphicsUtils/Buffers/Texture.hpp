#pragma once
#include "Graphics/GraphicsUtils/GpuResource.hpp"


#ifndef D_GRAPHICS_BUFFERS
#define D_GRAPHICS_BUFFERS Darius::Graphics::Utils::Buffers
#endif

namespace Darius::Graphics::Utils::Buffers
{
    class Texture : public GpuResource
    {
        friend class CommandContext;

    public:

        Texture() :
            mDepth(0u),
            mHeight(0u),
            mWidth(0u)
        { mCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN; }
        Texture(D3D12_CPU_DESCRIPTOR_HANDLE Handle) :
            mCpuDescriptorHandle(Handle),
            mDepth(0u),
            mHeight(0u),
            mWidth(0u) {}

        // Create a 1-level textures
        void Create2D(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitData);
        void CreateCube(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData);

        void CreateTGAFromMemory(const void* memBuffer, size_t fileSize, bool sRGB);
        bool CreateDDSFromMemory(const void* memBuffer, size_t fileSize, bool sRGB);
        void CreatePIXImageFromMemory(const void* memBuffer, size_t fileSize);

        virtual void Destroy() override
        {
            GpuResource::Destroy();
            mCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        }

        const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return mCpuDescriptorHandle; }

        uint32_t GetWidth() const { return mWidth; }
        uint32_t GetHeight() const { return mHeight; }
        uint32_t GetDepth() const { return mDepth; }

        bool    Is1D() const;
        bool    Is2D() const;
        bool    Is3D() const;
        UINT    ArraySize() const;
        bool    IsCubeMap() const;


#ifdef _D_EDITOR
        struct TextureMeta
        {
            size_t          Width = 0;
            size_t          Height = 0;     // Should be 1 for 1D textures
            size_t          Depth = 0;      // Should be 1 for 1D or 2D textures
            size_t          ArraySize = 0;  // For cubemap, this is a multiple of 6
            size_t          MipLevels = 0;
            uint32_t        MiscFlags = 0;
            uint32_t        MiscFlags2 = 0;
            DXGI_FORMAT     Format = DXGI_FORMAT_UNKNOWN;
            enum
                // Subset here matches D3D10_RESOURCE_DIMENSION and D3D11_RESOURCE_DIMENSION
            {
                TEX_DIMENSION_TEXTURE1D = 2,
                TEX_DIMENSION_TEXTURE2D = 3,
                TEX_DIMENSION_TEXTURE3D = 4,
            } Dimension;
            bool            Initialized = false;
        };

        TextureMeta const& GetMeta() const { return mMetaData; }

#endif // _D_EDITOR

        static inline std::string GetFormatString(DXGI_FORMAT format)
        {
            switch (format)
            {
            case DXGI_FORMAT_UNKNOWN:
                return "DXGI_FORMAT_UNKNOWN";
            case DXGI_FORMAT_R32G32B32A32_TYPELESS:
                return "DXGI_FORMAT_R32G32B32A32_TYPELESS";
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
                return "DXGI_FORMAT_R32G32B32A32_FLOAT";
            case DXGI_FORMAT_R32G32B32A32_UINT:
                return "DXGI_FORMAT_R32G32B32A32_UINT";
            case DXGI_FORMAT_R32G32B32A32_SINT:
                return "DXGI_FORMAT_R32G32B32A32_SINT";
            case DXGI_FORMAT_R32G32B32_TYPELESS:
                return "DXGI_FORMAT_R32G32B32_TYPELESS";
            case DXGI_FORMAT_R32G32B32_FLOAT:
                return "DXGI_FORMAT_R32G32B32_FLOAT";
            case DXGI_FORMAT_R32G32B32_UINT:
                return "DXGI_FORMAT_R32G32B32_UINT";
            case DXGI_FORMAT_R32G32B32_SINT:
                return "DXGI_FORMAT_R32G32B32_SINT";
            case DXGI_FORMAT_R16G16B16A16_TYPELESS:
                return "DXGI_FORMAT_R16G16B16A16_TYPELESS";
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
                return "DXGI_FORMAT_R16G16B16A16_FLOAT";
            case DXGI_FORMAT_R16G16B16A16_UNORM:
                return "DXGI_FORMAT_R16G16B16A16_UNORM";
            case DXGI_FORMAT_R16G16B16A16_UINT:
                return "DXGI_FORMAT_R16G16B16A16_UINT";
            case DXGI_FORMAT_R16G16B16A16_SNORM:
                return "DXGI_FORMAT_R16G16B16A16_SNORM";
            case DXGI_FORMAT_R16G16B16A16_SINT:
                return "DXGI_FORMAT_R16G16B16A16_SINT";
            case DXGI_FORMAT_R32G32_TYPELESS:
                return "DXGI_FORMAT_R32G32_TYPELESS";
            case DXGI_FORMAT_R32G32_FLOAT:
                return "DXGI_FORMAT_R32G32_FLOAT";
            case DXGI_FORMAT_R32G32_UINT:
                return "DXGI_FORMAT_R32G32_UINT";
            case DXGI_FORMAT_R32G32_SINT:
                return "DXGI_FORMAT_R32G32_SINT";
            case DXGI_FORMAT_R32G8X24_TYPELESS:
                return "DXGI_FORMAT_R32G8X24_TYPELESS";
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
                return "DXGI_FORMAT_D32_FLOAT_S8X24_UINT";
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
                return "DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS";
            case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
                return "DXGI_FORMAT_X32_TYPELESS_G8X24_UINT";
            case DXGI_FORMAT_R10G10B10A2_TYPELESS:
                return "DXGI_FORMAT_R10G10B10A2_TYPELESS";
            case DXGI_FORMAT_R10G10B10A2_UNORM:
                return "DXGI_FORMAT_R10G10B10A2_UNORM";
            case DXGI_FORMAT_R10G10B10A2_UINT:
                return "DXGI_FORMAT_R10G10B10A2_UINT";
            case DXGI_FORMAT_R11G11B10_FLOAT:
                return "DXGI_FORMAT_R11G11B10_FLOAT";
            case DXGI_FORMAT_R8G8B8A8_TYPELESS:
                return "DXGI_FORMAT_R8G8B8A8_TYPELESS";
            case DXGI_FORMAT_R8G8B8A8_UNORM:
                return "DXGI_FORMAT_R8G8B8A8_UNORM";
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
                return "DXGI_FORMAT_R8G8B8A8_UNORM_SRGB";
            case DXGI_FORMAT_R8G8B8A8_UINT:
                return "DXGI_FORMAT_R8G8B8A8_UINT";
            case DXGI_FORMAT_R8G8B8A8_SNORM:
                return "DXGI_FORMAT_R8G8B8A8_SNORM";
            case DXGI_FORMAT_R8G8B8A8_SINT:
                return "DXGI_FORMAT_R8G8B8A8_SINT";
            case DXGI_FORMAT_R16G16_TYPELESS:
                return "DXGI_FORMAT_R16G16_TYPELESS";
            case DXGI_FORMAT_R16G16_FLOAT:
                return "DXGI_FORMAT_R16G16_FLOAT";
            case DXGI_FORMAT_R16G16_UNORM:
                return "DXGI_FORMAT_R16G16_UNORM";
            case DXGI_FORMAT_R16G16_UINT:
                return "DXGI_FORMAT_R16G16_UINT";
            case DXGI_FORMAT_R16G16_SNORM:
                return "DXGI_FORMAT_R16G16_SNORM";
            case DXGI_FORMAT_R16G16_SINT:
                return "DXGI_FORMAT_R16G16_SINT";
            case DXGI_FORMAT_R32_TYPELESS:
                return "DXGI_FORMAT_R32_TYPELESS";
            case DXGI_FORMAT_D32_FLOAT:
                return "DXGI_FORMAT_D32_FLOAT";
            case DXGI_FORMAT_R32_FLOAT:
                return "DXGI_FORMAT_R32_FLOAT";
            case DXGI_FORMAT_R32_UINT:
                return "DXGI_FORMAT_R32_UINT";
            case DXGI_FORMAT_R32_SINT:
                return "DXGI_FORMAT_R32_SINT";
            case DXGI_FORMAT_R24G8_TYPELESS:
                return "DXGI_FORMAT_R24G8_TYPELESS";
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
                return "DXGI_FORMAT_D24_UNORM_S8_UINT";
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
                return "DXGI_FORMAT_R24_UNORM_X8_TYPELESS";
            case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
                return "DXGI_FORMAT_X24_TYPELESS_G8_UINT";
            case DXGI_FORMAT_R8G8_TYPELESS:
                return "DXGI_FORMAT_R8G8_TYPELESS";
            case DXGI_FORMAT_R8G8_UNORM:
                return "DXGI_FORMAT_R8G8_UNORM";
            case DXGI_FORMAT_R8G8_UINT:
                return "DXGI_FORMAT_R8G8_UINT";
            case DXGI_FORMAT_R8G8_SNORM:
                return "DXGI_FORMAT_R8G8_SNORM";
            case DXGI_FORMAT_R8G8_SINT:
                return "DXGI_FORMAT_R8G8_SINT";
            case DXGI_FORMAT_R16_TYPELESS:
                return "DXGI_FORMAT_R16_TYPELESS";
            case DXGI_FORMAT_R16_FLOAT:
                return "DXGI_FORMAT_R16_FLOAT";
            case DXGI_FORMAT_D16_UNORM:
                return "DXGI_FORMAT_D16_UNORM";
            case DXGI_FORMAT_R16_UNORM:
                return "DXGI_FORMAT_R16_UNORM";
            case DXGI_FORMAT_R16_UINT:
                return "DXGI_FORMAT_R16_UINT";
            case DXGI_FORMAT_R16_SNORM:
                return "DXGI_FORMAT_R16_SNORM";
            case DXGI_FORMAT_R16_SINT:
                return "DXGI_FORMAT_R16_SINT";
            case DXGI_FORMAT_R8_TYPELESS:
                return "DXGI_FORMAT_R8_TYPELESS";
            case DXGI_FORMAT_R8_UNORM:
                return "DXGI_FORMAT_R8_UNORM";
            case DXGI_FORMAT_R8_UINT:
                return "DXGI_FORMAT_R8_UINT";
            case DXGI_FORMAT_R8_SNORM:
                return "DXGI_FORMAT_R8_SNORM";
            case DXGI_FORMAT_R8_SINT:
                return "DXGI_FORMAT_R8_SINT";
            case DXGI_FORMAT_A8_UNORM:
                return "DXGI_FORMAT_A8_UNORM";
            case DXGI_FORMAT_R1_UNORM:
                return "DXGI_FORMAT_R1_UNORM";
            case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
                return "DXGI_FORMAT_R9G9B9E5_SHAREDEXP";
            case DXGI_FORMAT_R8G8_B8G8_UNORM:
                return "DXGI_FORMAT_R8G8_B8G8_UNORM";
            case DXGI_FORMAT_G8R8_G8B8_UNORM:
                return "DXGI_FORMAT_G8R8_G8B8_UNORM";
            case DXGI_FORMAT_BC1_TYPELESS:
                return "DXGI_FORMAT_BC1_TYPELESS";
            case DXGI_FORMAT_BC1_UNORM:
                return "DXGI_FORMAT_BC1_UNORM";
            case DXGI_FORMAT_BC1_UNORM_SRGB:
                return "DXGI_FORMAT_BC1_UNORM_SRGB";
            case DXGI_FORMAT_BC2_TYPELESS:
                return "DXGI_FORMAT_BC2_TYPELESS";
            case DXGI_FORMAT_BC2_UNORM:
                return "DXGI_FORMAT_BC2_UNORM";
            case DXGI_FORMAT_BC2_UNORM_SRGB:
                return "DXGI_FORMAT_BC2_UNORM_SRGB";
            case DXGI_FORMAT_BC3_TYPELESS:
                return "DXGI_FORMAT_BC3_TYPELESS";
            case DXGI_FORMAT_BC3_UNORM:
                return "DXGI_FORMAT_BC3_UNORM";
            case DXGI_FORMAT_BC3_UNORM_SRGB:
                return "DXGI_FORMAT_BC3_UNORM_SRGB";
            case DXGI_FORMAT_BC4_TYPELESS:
                return "DXGI_FORMAT_BC4_TYPELESS";
            case DXGI_FORMAT_BC4_UNORM:
                return "DXGI_FORMAT_BC4_UNORM";
            case DXGI_FORMAT_BC4_SNORM:
                return "DXGI_FORMAT_BC4_SNORM";
            case DXGI_FORMAT_BC5_TYPELESS:
                return "DXGI_FORMAT_BC5_TYPELESS";
            case DXGI_FORMAT_BC5_UNORM:
                return "DXGI_FORMAT_BC5_UNORM";
            case DXGI_FORMAT_BC5_SNORM:
                return "DXGI_FORMAT_BC5_SNORM";
            case DXGI_FORMAT_B5G6R5_UNORM:
                return "DXGI_FORMAT_B5G6R5_UNORM";
            case DXGI_FORMAT_B5G5R5A1_UNORM:
                return "DXGI_FORMAT_B5G5R5A1_UNORM";
            case DXGI_FORMAT_B8G8R8A8_UNORM:
                return "DXGI_FORMAT_B8G8R8A8_UNORM";
            case DXGI_FORMAT_B8G8R8X8_UNORM:
                return "DXGI_FORMAT_B8G8R8X8_UNORM";
            case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
                return "DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM";
            case DXGI_FORMAT_B8G8R8A8_TYPELESS:
                return "DXGI_FORMAT_B8G8R8A8_TYPELESS";
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
                return "DXGI_FORMAT_B8G8R8A8_UNORM_SRGB";
            case DXGI_FORMAT_B8G8R8X8_TYPELESS:
                return "DXGI_FORMAT_B8G8R8X8_TYPELESS";
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
                return "DXGI_FORMAT_B8G8R8X8_UNORM_SRGB";
            case DXGI_FORMAT_BC6H_TYPELESS:
                return "DXGI_FORMAT_BC6H_TYPELESS";
            case DXGI_FORMAT_BC6H_UF16:
                return "DXGI_FORMAT_BC6H_UF16";
            case DXGI_FORMAT_BC6H_SF16:
                return "DXGI_FORMAT_BC6H_SF16";
            case DXGI_FORMAT_BC7_TYPELESS:
                return "DXGI_FORMAT_BC7_TYPELESS";
            case DXGI_FORMAT_BC7_UNORM:
                return "DXGI_FORMAT_BC7_UNORM";
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                return "DXGI_FORMAT_BC7_UNORM_SRGB";
            case DXGI_FORMAT_AYUV:
                return "DXGI_FORMAT_AYUV";
            case DXGI_FORMAT_Y410:
                return "DXGI_FORMAT_Y410";
            case DXGI_FORMAT_Y416:
                return "DXGI_FORMAT_Y416";
            case DXGI_FORMAT_NV12:
                return "DXGI_FORMAT_NV12";
            case DXGI_FORMAT_P010:
                return "DXGI_FORMAT_P010";
            case DXGI_FORMAT_P016:
                return "DXGI_FORMAT_P016";
            case DXGI_FORMAT_420_OPAQUE:
                return "DXGI_FORMAT_420_OPAQUE";
            case DXGI_FORMAT_YUY2:
                return "DXGI_FORMAT_YUY2";
            case DXGI_FORMAT_Y210:
                return "DXGI_FORMAT_Y210";
            case DXGI_FORMAT_Y216:
                return "DXGI_FORMAT_Y216";
            case DXGI_FORMAT_NV11:
                return "DXGI_FORMAT_NV11";
            case DXGI_FORMAT_AI44:
                return "DXGI_FORMAT_AI44";
            case DXGI_FORMAT_IA44:
                return "DXGI_FORMAT_IA44";
            case DXGI_FORMAT_P8:
                return "DXGI_FORMAT_P8";
            case DXGI_FORMAT_A8P8:
                return "DXGI_FORMAT_A8P8";
            case DXGI_FORMAT_B4G4R4A4_UNORM:
                return "DXGI_FORMAT_B4G4R4A4_UNORM";
            case DXGI_FORMAT_P208:
                return "DXGI_FORMAT_P208";
            case DXGI_FORMAT_V208:
                return "DXGI_FORMAT_V208";
            case DXGI_FORMAT_V408:
                return "DXGI_FORMAT_V408";
            case DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
                return "DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE";
            case DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
                return "DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE";
            case DXGI_FORMAT_FORCE_UINT:
                return "DXGI_FORMAT_FORCE_UINT";
            default:
                return "";
            }
        }


    protected:

        uint32_t mWidth;
        uint32_t mHeight;
        uint32_t mDepth;

#if _D_EDITOR
        TextureMeta mMetaData;
#endif // _D_EDITOR

        D3D12_CPU_DESCRIPTOR_HANDLE mCpuDescriptorHandle;
    };
}





















































































































































































































































