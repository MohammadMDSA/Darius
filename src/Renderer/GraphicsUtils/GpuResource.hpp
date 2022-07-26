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


#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics
{
	class GraphicsContext;
	class ComputeContext;
}

namespace Darius::Graphics::Utils
{
	class GpuResource
	{
	public:
		friend class Darius::Graphics::GraphicsContext;
		friend class Darius::Graphics::ComputeContext;

		GpuResource() :
			mGpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
			mUsageState(D3D12_RESOURCE_STATE_COMMON),
			mTransitioningState((D3D12_RESOURCE_STATES)-1)
		{
		}

		GpuResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState) :
			mGpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
			mResource(resource),
			mUsageState(currentState),
			mTransitioningState((D3D12_RESOURCE_STATES)-1)
		{
		}

		~GpuResource() { Destroy(); }

		virtual void Destroy()
		{
			mResource = nullptr;
			mGpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
			mVersionID++;
		}

		ID3D12Resource* operator->() { return mResource.Get(); }
		const ID3D12Resource* operator->() const { return mResource.Get(); }

		ID3D12Resource* GetResource() { return mResource.Get(); }
		const ID3D12Resource* GetResource() const { return mResource.Get(); }

		ID3D12Resource** GetAddressOf() { return mResource.GetAddressOf(); }

		D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return mGpuVirtualAddress; }

		uint32_t GetVersionID() const { return mVersionID; }

	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource>		mResource;
		D3D12_RESOURCE_STATES						mUsageState;
		D3D12_RESOURCE_STATES						mTransitioningState;
		D3D12_GPU_VIRTUAL_ADDRESS					mGpuVirtualAddress;

		uint32_t									mVersionID = 0;
	};
}