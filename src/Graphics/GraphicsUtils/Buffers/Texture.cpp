#include "Graphics/pch.hpp"
#include "Texture.hpp"

#include "Graphics/CommandContext.hpp"
#include "Graphics/GraphicsDeviceManager.hpp"
#include "PixelBuffer.hpp"

#include <Utils/Common.hpp>

#include <DirectXTex.h>

namespace Darius::Graphics::Utils
{
	//--------------------------------------------------------------------------------------
	// Return the BPP for a particular format
	//--------------------------------------------------------------------------------------
	size_t BitsPerPixel(_In_ DXGI_FORMAT fmt);
}

namespace Darius::Graphics::Utils::Buffers
{

	static UINT BytesPerPixel(DXGI_FORMAT Format)
	{
		return (UINT)BitsPerPixel(Format) / 8;
	};

	Texture::Texture() :
		GpuResource()
	{
		mCpuDescriptorHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	Texture::Texture(D3D12_CPU_DESCRIPTOR_HANDLE Handle) :
		GpuResource(),
		mCpuDescriptorHandle(Handle)
	{ }

	void Texture::Create2D(DirectX::ScratchImage const& image, DirectX::TexMetadata const& meta, DXGI_FORMAT format)
	{
		D_ASSERT_M(meta.depth == 1, "2D Texture does not support depth > 1");
		D_ASSERT(meta.dimension == DirectX::TEX_DIMENSION_TEXTURE2D);

		Destroy();

		mUsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		mMetaData.Width = meta.width;
		mMetaData.Height = meta.height;
		mMetaData.Depth = meta.depth;
		mMetaData.ArraySize = meta.arraySize;
		mMetaData.MipLevels = meta.mipLevels;
		mMetaData.MiscFlags = meta.miscFlags;
		mMetaData.MiscFlags2 = meta.miscFlags2;
		mMetaData.Format = format;
		mMetaData.Initialized = true;

		D3D12ResourceDesc texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = (UINT64)mMetaData.Width;
		texDesc.Height = (UINT)mMetaData.Height;
		texDesc.DepthOrArraySize = (UINT16)mMetaData.ArraySize;
		texDesc.MipLevels = (UINT16)mMetaData.MipLevels;
		texDesc.Format = mMetaData.Format;
		texDesc.SampleDesc.Count = 1u;
		texDesc.SampleDesc.Quality = 0u;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texDesc.Alignment = 0ull;
		texDesc.ReservedResource = false;

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D_HR_CHECK(CreateCommittedResource(D_GRAPHICS_DEVICE::GetDevice(), texDesc, HeapProps, D3D12_HEAP_FLAG_NONE, mUsageState, nullptr));

		GetResource()->SetName(L"Texture");

		UINT subResCount = (UINT)(mMetaData.ArraySize * mMetaData.MipLevels);
		D3D12_SUBRESOURCE_DATA* subData = new D3D12_SUBRESOURCE_DATA[subResCount];

		int index = 0;
		for(size_t arrIdx = 0; arrIdx < mMetaData.ArraySize; arrIdx++)
		{
			for(size_t mipIdx = 0; mipIdx < mMetaData.MipLevels; mipIdx++)
			{
				auto& sub = subData[index];
				auto img = image.GetImage(mipIdx, arrIdx, 0);
				sub.pData = img->pixels;
				sub.RowPitch = img->rowPitch;
				sub.SlicePitch = img->slicePitch;

				index++;
			}
		}

		CommandContext::InitializeTexture(*this, subResCount, subData);
		delete[] subData;

		if(mCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC srv;
		srv.Format = format;
		srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if(meta.arraySize > 1)
		{
			srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srv.Texture2D.MipLevels = (UINT)meta.mipLevels;
			srv.Texture2D.MostDetailedMip = 0;
			srv.Texture2D.PlaneSlice = 0;
			srv.Texture2D.ResourceMinLODClamp = 0.f;
		}
		else
		{
			srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
			srv.Texture2DArray.ArraySize = (UINT)meta.arraySize;
			srv.Texture2DArray.FirstArraySlice = 0;
			srv.Texture2DArray.MipLevels = (!meta.mipLevels) ? -1 : texDesc.MipLevels;
			srv.Texture2DArray.MostDetailedMip = 0;
			srv.Texture2DArray.PlaneSlice = 0;
			srv.Texture2DArray.ResourceMinLODClamp = 0.f;
		}
		GetParentDevice()->CreateShaderResourceView(GetResource(), &srv, mCpuDescriptorHandle);
		mMetaData.MiscFlags = 0;

	}

	void Texture::CreateCube(DirectX::ScratchImage const& image, DirectX::TexMetadata const& meta, DXGI_FORMAT format)
	{
		D_ASSERT(meta.IsCubemap() && meta.dimension == DirectX::TEX_DIMENSION_TEXTURE2D);

		mUsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12ResourceDesc texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = (UINT64)meta.width;
		texDesc.Height = (UINT)meta.height;
		texDesc.DepthOrArraySize = (UINT16)meta.arraySize;
		texDesc.MipLevels = (UINT16)meta.mipLevels;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1u;
		texDesc.SampleDesc.Quality = 0ull;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texDesc.ReservedResource = false;

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D_HR_CHECK(CreateCommittedResource(D_GRAPHICS_DEVICE::GetDevice(), texDesc, HeapProps, D3D12_HEAP_FLAG_NONE, mUsageState, nullptr));

		GetResource()->SetName(L"Texture");

		UINT subResCount = (UINT)(meta.arraySize * meta.mipLevels);
		D3D12_SUBRESOURCE_DATA* subData = new D3D12_SUBRESOURCE_DATA[subResCount];

		int index = 0;
		for(size_t arrIdx = 0; arrIdx < meta.arraySize; arrIdx++)
		{
			for(size_t mipIdx = 0; mipIdx < meta.mipLevels; mipIdx++)
			{
				auto& sub = subData[index];
				auto img = image.GetImage(mipIdx, arrIdx, 0);
				sub.pData = img->pixels;
				sub.RowPitch = img->rowPitch;
				sub.SlicePitch = img->slicePitch;

				index++;
			}
		}

		CommandContext::InitializeTexture(*this, subResCount, subData);
		delete[] subData;

		if(mCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		if(meta.arraySize > 6)
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
			srvDesc.TextureCubeArray.MipLevels = (!meta.mipLevels) ? -1 : texDesc.MipLevels;
			srvDesc.TextureCubeArray.MostDetailedMip = 0;
			srvDesc.TextureCubeArray.NumCubes = (UINT)meta.arraySize / 6;
			srvDesc.TextureCubeArray.ResourceMinLODClamp = 0.f;
		}
		else
		{
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = (!meta.mipLevels) ? -1 : texDesc.MipLevels;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		}
		GetParentDevice()->CreateShaderResourceView(GetResource(), &srvDesc, mCpuDescriptorHandle);

		mMetaData.Width = meta.width;
		mMetaData.Height = meta.height;
		mMetaData.Depth = meta.depth;
		mMetaData.ArraySize = meta.arraySize;
		mMetaData.MipLevels = meta.mipLevels;
		mMetaData.MiscFlags |= DirectX::TEX_MISC_TEXTURECUBE;
		mMetaData.Format = format;
		mMetaData.Dimension = TextureMeta::TEX_DIMENSION_TEXTURE2D;
		mMetaData.Initialized = true;

	}

	void Texture::Create2D(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData)
	{
		Destroy();

		mUsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		mMetaData.Width = Width;
		mMetaData.Height = Height;
		mMetaData.Depth = 1;
		mMetaData.ArraySize = 0;
		mMetaData.MipLevels = 1;
		mMetaData.MiscFlags = 0;
		mMetaData.Format = Format;
		mMetaData.Dimension = TextureMeta::TEX_DIMENSION_TEXTURE2D;
		mMetaData.MiscFlags = 0;
		mMetaData.Initialized = true;

		D3D12ResourceDesc texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = Width;
		texDesc.Height = (UINT)Height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = Format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texDesc.ReservedResource = false;

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D_HR_CHECK(CreateCommittedResource(D_GRAPHICS_DEVICE::GetDevice(), texDesc, HeapProps, D3D12_HEAP_FLAG_NONE, mUsageState, nullptr));

		GetResource()->SetName(L"Texture");

		D3D12_SUBRESOURCE_DATA texResource;
		texResource.pData = InitialData;
		texResource.RowPitch = RowPitchBytes;
		texResource.SlicePitch = RowPitchBytes * Height;

		CommandContext::InitializeTexture(*this, 1, &texResource);

		if(mCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		GetParentDevice()->CreateShaderResourceView(GetResource(), nullptr, mCpuDescriptorHandle);
	}

	void Texture::CreateCube(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData)
	{
		Destroy();

		mUsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12ResourceDesc texDesc = {};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = Width;
		texDesc.Height = (UINT)Height;
		texDesc.DepthOrArraySize = 6;
		texDesc.MipLevels = 1;
		texDesc.Format = Format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texDesc.ReservedResource = false;

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		D_HR_CHECK(CreateCommittedResource(D_GRAPHICS_DEVICE::GetDevice(), texDesc, HeapProps, D3D12_HEAP_FLAG_NONE, mUsageState, nullptr));

		GetResource()->SetName(L"Texture");

		D3D12_SUBRESOURCE_DATA texResource;
		texResource.pData = InitialData;
		texResource.RowPitch = RowPitchBytes;
		texResource.SlicePitch = texResource.RowPitch * Height;

		CommandContext::InitializeTexture(*this, 1, &texResource);

		if(mCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		GetParentDevice()->CreateShaderResourceView(GetResource(), &srvDesc, mCpuDescriptorHandle);

		mMetaData.Width = Width;
		mMetaData.Height = Height;
		mMetaData.Depth = 6;
		mMetaData.ArraySize = 6;
		mMetaData.MipLevels = 1;
		mMetaData.MiscFlags |= DirectX::TEX_MISC_TEXTURECUBE;
		mMetaData.Format = Format;
		mMetaData.Dimension = TextureMeta::TEX_DIMENSION_TEXTURE2D;
		mMetaData.Initialized = true;
	}


	void Texture::CreateTGAFromMemory(const void* _filePtr, size_t, bool sRGB)
	{
		const uint8_t* filePtr = (const uint8_t*)_filePtr;

		// Skip first two bytes
		filePtr += 2;

		/*uint8_t imageTypeCode =*/ *filePtr++;

		// Ignore another 9 bytes
		filePtr += 9;

		uint16_t imageWidth = *(uint16_t*)filePtr;
		filePtr += sizeof(uint16_t);
		uint16_t imageHeight = *(uint16_t*)filePtr;
		filePtr += sizeof(uint16_t);
		uint8_t bitCount = *filePtr++;

		// Ignore another byte
		filePtr++;

		uint32_t* formattedData = new uint32_t[imageWidth * imageHeight];
		uint32_t* iter = formattedData;

		uint8_t numChannels = bitCount / 8;
		uint32_t numBytes = imageWidth * imageHeight * numChannels;

		switch(numChannels)
		{
		default:
			break;
		case 3:
			for(uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 3)
			{
				*iter++ = 0xff000000 | filePtr[0] << 16 | filePtr[1] << 8 | filePtr[2];
				filePtr += 3;
			}
			break;
		case 4:
			for(uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 4)
			{
				*iter++ = filePtr[3] << 24 | filePtr[0] << 16 | filePtr[1] << 8 | filePtr[2];
				filePtr += 4;
			}
			break;
		}

		Create2D(4 * imageWidth, imageWidth, imageHeight, sRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM, formattedData);

		delete[] formattedData;
	}

	bool Texture::CreateDDSFromMemory(const void* filePtr, size_t fileSize, bool sRGB)
	{
#if 0
		if(mCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		HRESULT hr = CreateDDSTextureFromMemory(D_GRAPHICS_DEVICE::GetDevice(),
			(const uint8_t*)filePtr, fileSize, 0, sRGB, mResource.ReleaseAndGetAddressOf(), mCpuDescriptorHandle);

		DirectX::TexMetadata meta;
		DirectX::GetMetadataFromDDSMemory(filePtr, fileSize, DirectX::DDS_FLAGS_NONE, meta);

		memcpy(&mMetaData, &meta, sizeof(DirectX::TexMetadata));

		mMetaData.Initialized = true;

		return SUCCEEDED(hr);
#else
		DirectX::DDS_FLAGS flags = DirectX::DDS_FLAGS::DDS_FLAGS_NONE;

		DirectX::TexMetadata meta;
		DirectX::ScratchImage img;
		HRESULT hr = DirectX::LoadFromDDSMemory(filePtr, fileSize, flags, &meta, img);
		DXGI_FORMAT usedFormat = sRGB ? PixelBuffer::MakeSRGB(meta.format) : meta.format;

		if(meta.IsCubemap())
			CreateCube(img, meta, usedFormat);
		else
			Create2D(img, meta, usedFormat);

		return SUCCEEDED(hr);
#endif
}

	bool Texture::CreateWICFromMemory(const void* memBuffer, size_t fileSize, bool sRGB)
	{
		DirectX::WIC_FLAGS flags = DirectX::WIC_FLAGS::WIC_FLAGS_NONE;
		if(sRGB)
			flags |= DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_SRGB;

		DirectX::TexMetadata meta;
		DirectX::ScratchImage img;
		HRESULT hr = DirectX::LoadFromWICMemory(memBuffer, fileSize, flags, &meta, img);
		Create2D(img.GetPixelsSize() / meta.height, meta.width, meta.height, meta.format, img.GetPixels());
		memcpy(&mMetaData, &meta, sizeof(DirectX::TexMetadata));
		mMetaData.Initialized = true;

		return SUCCEEDED(hr);
	}

	void Texture::CreatePIXImageFromMemory(const void* memBuffer, size_t fileSize)
	{
		struct Header
		{
			DXGI_FORMAT Format;
			uint32_t Pitch;
			uint32_t Width;
			uint32_t Height;
		};
		const Header& header = *(Header*)memBuffer;

		D_ASSERT_M(fileSize >= header.Pitch * BytesPerPixel(header.Format) * header.Height + sizeof(Header),
			"Raw PIX image dump has an invalid file size");

		Create2D(header.Pitch, header.Width, header.Height, header.Format, (uint8_t*)memBuffer + sizeof(Header));
	}

	bool Texture::IsCubeMap() const
	{
		return (mMetaData.MiscFlags & DirectX::TEX_MISC_TEXTURECUBE) != 0;
	}
}
