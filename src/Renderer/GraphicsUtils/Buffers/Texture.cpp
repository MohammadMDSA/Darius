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

#include "Renderer/pch.hpp"
#include "Texture.hpp"

#include "Renderer/CommandContext.hpp"
#include "Renderer/GraphicsUtils/DDSTextureLoader.hpp"
#include "Renderer/RenderDeviceManager.hpp"
#include "Renderer/Resources/TextureResource.hpp"

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

	void Texture::Create2D(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData)
	{
		Destroy();

		mUsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		mWidth = (uint32_t)Width;
		mHeight = (uint32_t)Height;
		mDepth = 1;

		D3D12_RESOURCE_DESC texDesc = {};
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

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		auto device = D_RENDERER_DEVICE::GetDevice();

		D_HR_CHECK(device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			mUsageState, nullptr, IID_PPV_ARGS(mResource.ReleaseAndGetAddressOf())));

		mResource->SetName(L"Texture");

		D3D12_SUBRESOURCE_DATA texResource;
		texResource.pData = InitialData;
		texResource.RowPitch = RowPitchBytes;
		texResource.SlicePitch = RowPitchBytes * Height;

		CommandContext::InitializeTexture(*this, 1, &texResource);

		if (mCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(mResource.Get(), nullptr, mCpuDescriptorHandle);
	}

	void Texture::CreateCube(size_t RowPitchBytes, size_t Width, size_t Height, DXGI_FORMAT Format, const void* InitialData)
	{
		Destroy();

		mUsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		mWidth = (uint32_t)Width;
		mHeight = (uint32_t)Height;
		mDepth = 6;

		D3D12_RESOURCE_DESC texDesc = {};
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

		D3D12_HEAP_PROPERTIES HeapProps;
		HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		HeapProps.CreationNodeMask = 1;
		HeapProps.VisibleNodeMask = 1;

		auto device = D_RENDERER_DEVICE::GetDevice();

		D_HR_CHECK(device->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &texDesc,
			mUsageState, nullptr, IID_PPV_ARGS(mResource.ReleaseAndGetAddressOf())));

		mResource->SetName(L"Texture");

		D3D12_SUBRESOURCE_DATA texResource;
		texResource.pData = InitialData;
		texResource.RowPitch = RowPitchBytes;
		texResource.SlicePitch = texResource.RowPitch * Height;

		CommandContext::InitializeTexture(*this, 1, &texResource);

		if (mCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = Format;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		device->CreateShaderResourceView(mResource.Get(), &srvDesc, mCpuDescriptorHandle);
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

		switch (numChannels)
		{
		default:
			break;
		case 3:
			for (uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 3)
			{
				*iter++ = 0xff000000 | filePtr[0] << 16 | filePtr[1] << 8 | filePtr[2];
				filePtr += 3;
			}
			break;
		case 4:
			for (uint32_t byteIdx = 0; byteIdx < numBytes; byteIdx += 4)
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
		if (mCpuDescriptorHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mCpuDescriptorHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		HRESULT hr = CreateDDSTextureFromMemory(D_RENDERER_DEVICE::GetDevice(),
			(const uint8_t*)filePtr, fileSize, 0, sRGB, &mResource, mCpuDescriptorHandle);

		DirectX::TexMetadata meta;
		DirectX::GetMetadataFromDDSMemory(filePtr, fileSize, DirectX::DDS_FLAGS_NONE, meta);

#ifdef _D_EDITOR
		memcpy(&mMetaData, &meta, sizeof(DirectX::TexMetadata));
		mMetaData.Initialized = true;
#endif // _D_EDITOR

		mWidth = meta.width;
		mHeight = meta.height;
		mDepth = meta.depth;

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

}
