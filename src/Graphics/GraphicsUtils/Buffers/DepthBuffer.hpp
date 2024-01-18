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

#ifndef D_GRAPHICS_BUFFERS
#define D_GRAPHICS_BUFFERS Darius::Graphics::Utils::Buffers
#endif

namespace Darius::Graphics::Utils::Buffers
{
	class DepthBuffer : public PixelBuffer
	{
	public:
		DepthBuffer(float ClearDepth = 0.0f, uint8_t ClearStencil = 0) :
			mClearDepth(ClearDepth),
			mClearStencil(ClearStencil),
			mHasStencil(false)
		{
			mDSV[0].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
			mDSV[1].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
			mDSV[2].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
			mDSV[3].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
			mDepthSRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
			mStencilSRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		}

		// Create a depth buffer.  If an address is supplied, memory will not be allocated.
		// The vmem address allows you to alias buffers (which can be especially useful for
		// reusing ESRAM across a frame.)
		void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, DXGI_FORMAT Format,
			D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

		void Create(const std::wstring& Name, uint32_t Width, uint32_t Height, uint32_t NumSamples, DXGI_FORMAT Format,
			D3D12_GPU_VIRTUAL_ADDRESS VidMemPtr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

		// Get pre-created CPU-visible descriptor handles
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV() const { return mDSV[0]; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_DepthReadOnly() const { return mDSV[1]; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_StencilReadOnly() const { return mDSV[2]; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetDSV_ReadOnly() const { return mDSV[3]; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSRV() const { return mDepthSRV; }
		const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSRV() const { return mStencilSRV; }

		float GetClearDepth() const { return mClearDepth; }
		uint8_t GetClearStencil() const { return mClearStencil; }
		bool HasStencil() const { return (bool)mHasStencil; }

	protected:

		void CreateDerivedViews(ID3D12Device* Device, DXGI_FORMAT Format);

		float mClearDepth;
		D3D12_CPU_DESCRIPTOR_HANDLE mDSV[4];
		D3D12_CPU_DESCRIPTOR_HANDLE mDepthSRV;
		D3D12_CPU_DESCRIPTOR_HANDLE mStencilSRV;
		uint8_t mClearStencil;
		uint8_t mHasStencil : 1;
	};
}