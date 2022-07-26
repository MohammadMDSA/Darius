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

#include "PixelBuffer.hpp"

#include <Math/Color.hpp>
#include <Utils/Assert.hpp>

#ifndef D_GRAPHICS_BUFFERS
#define D_GRAPHICS_BUFFERS Darius::Graphics::Utils::Buffers
#endif

namespace Darius::Graphics
{
	class CommandContext;
}

using namespace Darius::Graphics;

namespace Darius::Graphics::Utils::Buffers
{
	class ColorBuffer : public PixelBuffer
	{
	public:
		ColorBuffer(D_MATH::Color clearColor = D_MATH::Color(0.f, 0.f, 0.f, 0.f)) :
			mClearColor(clearColor),
			mNumMipMaps(0),
			mFragmentCount(1),
			mSampleCount(1)
		{
			mRtvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
			mSrvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
			for (int i = 0; i < _countof(mUavHandle); i++)
				mUavHandle[i].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		}

		// Create a color buffer from a swap chain buffer. Unordered access is restricted.
		void CreateFromSwapChain(const std::wstring& name, ID3D12Resource* baseResouces);

		// Create a color buffer. If an address is supplied, memory will not be allocated
		// The vmem address allows you to alias buffers (which can be especially useful for
		// reusing ESRAM across a frame.).
		void Create(const std::wstring& name, uint32_t width, uint32_t height, uint32_t numMips, DXGI_FORMAT format, D3D12_GPU_VIRTUAL_ADDRESS vidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

		// Create a color buffer. If an address is supplied, memory will not be allocated.
		// The vmem address allows you to alias buffers (which can be especially useful for reusing ESRAM across a frame.).
		void CreateArray(const std::wstring& name, uint32_t width, uint32_t height, uint32_t arrayCount, DXGI_FORMAT format, D3D12_GPU_VIRTUAL_ADDRESS vidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

		// Get pre-created CPU-visible descriptor handles
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return mSrvHandle; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV(void) const { return mRtvHandle; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return mUavHandle[0]; }

		void SetClearColor(D_MATH::Color clearColor) { mClearColor = clearColor; }

		void SetMsaaMode(uint32_t numColorSamples, uint32_t numCoverageSamples)
		{
			D_ASSERT(numCoverageSamples >= numColorSamples);
			mFragmentCount = numColorSamples;
			mSampleCount = numCoverageSamples;
		}

		D_MATH::Color GetClearColor(void) const { return mClearColor; }

		// This will work for all texture sizes, but it's recommended for speed and quality
		// That you use dimensions with powers of two (but no necessarily square.) Pass
		// 0 for ArrayCount to reserve space for mips at creation time.
		void GenerateMipMaps(CommandContext& BaseContext);

	protected:

		D3D12_RESOURCE_FLAGS CombineResourceFlags(void) const
		{
			D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

			if (flags == D3D12_RESOURCE_FLAG_NONE && mFragmentCount == 1)
				flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

			return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | flags;
		}

		// Compute the number of texture levels need to reduce to 1x1. This uses
		// _BitScanReverse to find the highest set bit. Each dimension reduces by
		// half and truncates bits. The dimension 256 (0x100) has 9 mip levels, same
		// as the dimension 511 (0x1FF)
		static inline uint32_t ComputeNumMips(uint32_t width, uint32_t height)
		{
			uint32_t hightBit;
			_BitScanReverse((unsigned long*)&hightBit, width | height);
			return hightBit + 1;
		}

		void CreateDerivedViews(ID3D12Device* device, DXGI_FORMAT format, uint32_t arraySize, uint32_t numMips = 1);

		D_MATH::Color mClearColor;
		D3D12_CPU_DESCRIPTOR_HANDLE mSrvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE mRtvHandle;
		D3D12_CPU_DESCRIPTOR_HANDLE mUavHandle[12];
		uint32_t mNumMipMaps; // number of texture sublevels
		uint32_t mFragmentCount;
		uint32_t mSampleCount;
	};
}