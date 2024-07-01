#pragma once

#include "Graphics/GraphicsUtils/GpuResource.hpp"

#ifndef D_GRAPHICS_BUFFERS
#define D_GRAPHICS_BUFFERS Darius::Graphics::Utils::Buffers
#endif

namespace Darius::Graphics::Utils::Buffers
{
	class PixelBuffer : public GpuResource
	{
	public:
		PixelBuffer() :
			mWidth(0),
			mHeight(0),
			mArraySize(0),
			mFormat(DXGI_FORMAT_UNKNOWN),
			mBankRotation(0)
		{}

		uint32_t GetWidth(void) const { return mWidth; }
		uint32_t GetHeight(void) const { return mHeight; }
		uint32_t GetDepth(void) const { return mArraySize; }
		const DXGI_FORMAT& GetFormat(void) const { return mFormat; };

		// Has no effect on Desktop
		void SetBankRotation(uint32_t rotationAmount)
		{
			(rotationAmount);
		}

		// Write the raw pixel buffer contents to a file
		// Note that data is preceded by a 16-byte header: { DXGI_FORMAT, Pitch (in pixels), Width (in pixels), Height }
		void ExportToFile(const std::wstring& filePath);

		static size_t BytesPerPixel(DXGI_FORMAT format);


		static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT format);
		static DXGI_FORMAT GetUAVFormat(DXGI_FORMAT format);
		static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format);
		static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT format);
		static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT format);
		INLINE static DXGI_FORMAT MakeSRGB(DXGI_FORMAT format)
		{
			switch(format)
			{
			case DXGI_FORMAT_R8G8B8A8_UNORM:
				return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

			case DXGI_FORMAT_BC1_UNORM:
				return DXGI_FORMAT_BC1_UNORM_SRGB;

			case DXGI_FORMAT_BC2_UNORM:
				return DXGI_FORMAT_BC2_UNORM_SRGB;

			case DXGI_FORMAT_BC3_UNORM:
				return DXGI_FORMAT_BC3_UNORM_SRGB;

			case DXGI_FORMAT_B8G8R8A8_UNORM:
				return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

			case DXGI_FORMAT_B8G8R8X8_UNORM:
				return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

			case DXGI_FORMAT_BC7_UNORM:
				return DXGI_FORMAT_BC7_UNORM_SRGB;

			default:
				return format;
			}
		}

	protected:

		D3D12_RESOURCE_DESC DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, DXGI_FORMAT format, UINT flags);
		D3D12_RESOURCE_DESC DescribeTex3D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, DXGI_FORMAT format, UINT flags);

		void AssociateWithResource(ID3D12Device* device, const std::wstring& name, ID3D12Resource* resource, D3D12_RESOURCE_STATES currentsState);

		void CreateTextureResource(ID3D12Device* device, const std::wstring& name, const D3D12_RESOURCE_DESC& resouceDesc, D3D12_CLEAR_VALUE clearValue, D3D12_GPU_VIRTUAL_ADDRESS vidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN, D3D12_RESOURCE_STATES startState = D3D12_RESOURCE_STATE_COMMON);

		uint32_t mWidth;
		uint32_t mHeight;
		uint32_t mArraySize;
		DXGI_FORMAT mFormat;
		uint32_t mBankRotation;
	};
}