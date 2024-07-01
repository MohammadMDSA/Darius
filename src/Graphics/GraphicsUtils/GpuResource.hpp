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

#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace Darius::Graphics
{
	class GraphicsContext;
	class ComputeContext;
	class CommandContext;
}

namespace Darius::Graphics::Utils
{
	struct D3D12ResourceDesc : public D3D12_RESOURCE_DESC
	{
		D3D12ResourceDesc() = default;

		D3D12ResourceDesc(D3D12_RESOURCE_DESC const& other) :
			D3D12_RESOURCE_DESC(other)
		{ }

		bool ReservedResource = false;
	};

	class GpuResource
	{
	public:
		friend class Darius::Graphics::GraphicsContext;
		friend class Darius::Graphics::ComputeContext;
		friend class Darius::Graphics::CommandContext;

		GpuResource() :
			mUsageState(D3D12_RESOURCE_STATE_COMMON),
			mTransitioningState((D3D12_RESOURCE_STATES)-1),
			mGpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
			mParentDevice(nullptr)
		{ }

		GpuResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState) :
			mResource(resource),
			mUsageState(currentState),
			mTransitioningState((D3D12_RESOURCE_STATES)-1),
			mGpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL)
		{
			D_ASSERT(resource);
			resource->GetDevice(IID_PPV_ARGS(mParentDevice.GetAddressOf()));
		}

		GpuResource(GpuResource const&) = default;

		D3D12_RESOURCE_STATES GetUsageState() const { return mUsageState; }

		~GpuResource() { Destroy(); }

		GpuResource& operator= (GpuResource const&) = default;

		virtual void Destroy()
		{
			mResource = nullptr;
			mGpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
			mVersionID++;
		}

		INLINE ID3D12Resource* operator->() { return mResource.Get(); }
		INLINE const ID3D12Resource* operator->() const { return mResource.Get(); }

		INLINE ID3D12Resource* GetResource() const { return mResource.Get(); }

		INLINE ID3D12Resource** GetAddressOf() { return mResource.GetAddressOf(); }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return mGpuVirtualAddress; }

		INLINE uint32_t GetVersionID() const { return mVersionID; }

		INLINE ID3D12Device* GetParentDevice() const { return mParentDevice.Get(); }

		INLINE D3D12ResourceDesc GetDesc() const { return mDesc; }

	protected:
		HRESULT CreateCommittedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, D3D12_HEAP_PROPERTIES const& heapProps, D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue);

		HRESULT CreatePlacedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, ID3D12Heap* heap, uint64_t heapOffset, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue);

		void AttachOther(ID3D12Resource* other);

		D3D12_RESOURCE_STATES						mUsageState;
		D3D12_RESOURCE_STATES						mTransitioningState;
		D3D12_GPU_VIRTUAL_ADDRESS					mGpuVirtualAddress;

	private:
		Microsoft::WRL::ComPtr<ID3D12Resource>		mResource;
		D3D12ResourceDesc							mDesc;

		Microsoft::WRL::ComPtr<ID3D12Device>		mParentDevice;

		uint32_t									mVersionID = 0;
	};
}