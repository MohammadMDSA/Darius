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

#include <Core/RefCounting/Counted.hpp>
#include <Core/RefCounting/Ref.hpp>
#include <Core/Containers/Vector.hpp>
#include <Utils/Assert.hpp>
#include <Utils/Common.hpp>

#ifndef D_GRAPHICS_UTILS
#define D_GRAPHICS_UTILS Darius::Graphics::Utils
#endif

namespace D3DX12Residency
{
	class ManagedObject;
}

namespace Darius::Graphics
{
	class GraphicsContext;
	class ComputeContext;
	class CommandContext;
}

namespace Darius::Graphics::Utils
{

	INLINE bool IsGPUOnly(D3D12_HEAP_TYPE heapType, const D3D12_HEAP_PROPERTIES* customHeapProperties = nullptr)
	{
		D_ASSERT(heapType == D3D12_HEAP_TYPE_CUSTOM ? customHeapProperties != nullptr : true);
		return heapType == D3D12_HEAP_TYPE_DEFAULT ||
			(heapType == D3D12_HEAP_TYPE_CUSTOM &&
				(customHeapProperties->CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE));
	}

	class D3D12Heap : public D_CORE::Counted
	{
	public:
		D3D12Heap(ID3D12Device* Parent);
		~D3D12Heap();

		virtual void Destroy();

		inline ID3D12Heap* GetHeap() const { return mHeap.Get(); }
		void SetHeap(ID3D12Heap* HeapIn, std::string const& InName, bool Track = true, bool bForceGetGPUAddress = false);

		void BeginTrackingResidency(size_t Size);
		void DisallowTrackingResidency();

		inline D3D12_HEAP_DESC GetHeapDesc() const { return mHeapDesc; }
		inline D_CONTAINERS::DVector<D3DX12Residency::ManagedObject*> GetResidencyHandles()
		{
			if(mRequiresResidencyTracking)
			{
				D_ASSERT_M(mResidencyHandle, "Resource requires residency tracking, but BeginTrackingResidency() was not called.");
				return {mResidencyHandle};
			}
			else
			{
				return {};
			}
		}
		inline D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return mGPUVirtualAddress; }
		inline void SetIsTransient(bool bInIsTransient) { mIsTransient = bInIsTransient; }
		inline bool GetIsTransient() const { return mIsTransient; }

		virtual bool Release() override
		{
			Destroy();
			return true;
		}

	private:

		Microsoft::WRL::ComPtr<ID3D12Heap> mHeap;
		D3D12_HEAP_DESC mHeapDesc;
		D3D12_GPU_VIRTUAL_ADDRESS mGPUVirtualAddress = 0;
		D3DX12Residency::ManagedObject* mResidencyHandle = nullptr; // Residency handle owned by this object

		ID3D12Device* mParentDevice;

		uint8_t mTrack : 1 = true;
		uint8_t mIsTransient : 1 = false; // Whether this is a transient heap
		uint8_t mRequiresResidencyTracking : 1 = true;
	};


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
			mParentDevice(nullptr),
			mResidencyHandle(nullptr)
		{
			InitializeInternal();
		}

		GpuResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState) :
			mResource(resource),
			mUsageState(currentState),
			mTransitioningState((D3D12_RESOURCE_STATES)-1),
			mGpuVirtualAddress(D3D12_GPU_VIRTUAL_ADDRESS_NULL),
			mResidencyHandle(nullptr)
		{
			D_ASSERT(resource);
			resource->GetDevice(IID_PPV_ARGS(mParentDevice.GetAddressOf()));
			InitializeInternal();
		}

		GpuResource(GpuResource const&) = default;

		D3D12_RESOURCE_STATES GetUsageState() const { return mUsageState; }

		virtual ~GpuResource() { Destroy(); }

		GpuResource& operator= (GpuResource const&) = default;

		virtual void Destroy();

		void StartTrackingForResidency();

		INLINE ID3D12Resource* operator->() { return mResource.Get(); }
		INLINE const ID3D12Resource* operator->() const { return mResource.Get(); }

		INLINE ID3D12Resource* GetResource() const { return mResource.Get(); }

		INLINE ID3D12Resource** GetAddressOf() { return mResource.GetAddressOf(); }

		INLINE D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress() const { return mGpuVirtualAddress; }

		INLINE uint32_t GetVersionID() const { return mVersionID; }

		INLINE ID3D12Device* GetParentDevice() const { return mParentDevice.Get(); }

		INLINE D3D12ResourceDesc GetDesc() const { return mDesc; }

		INLINE bool IsReservedResource() const { return mDesc.ReservedResource; }
		INLINE bool IsPlacedResource() const { return !mBackingHeap.IsNull(); }

		INLINE D_CONTAINERS::DVector<D3DX12Residency::ManagedObject*> GetResidencyHandles()
		{
			if(!mRequiresResidencyTracking)
			{
				return {};
			}
			else if(IsPlacedResource())
			{
				return mBackingHeap->GetResidencyHandles();
			}
			else if(IsReservedResource())
			{
				D_ASSERT_NOENTRY(); // Not implemented yet
			}
			else
			{
				D_ASSERT_M(mResidencyHandle, "Resource requires residency tracking, but StartTrackingForResidency() was not called.");
				return {mResidencyHandle};
			}
		}

	protected:
		void InitializeInternal();

		HRESULT CreateCommittedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, D3D12_HEAP_PROPERTIES const& heapProps, D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue);

		HRESULT CreatePlacedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, D3D12Heap* heap, uint64_t heapOffset, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue);

		HRESULT CreateResesrvedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue);

		void CommitReservedResource(ID3D12CommandQueue* queue, uint64_t requiredCommitSizeInBytes);

		void AttachOther(ID3D12Resource* other);

		D3D12_RESOURCE_STATES						mUsageState;
		D3D12_RESOURCE_STATES						mTransitioningState;
		D3D12_GPU_VIRTUAL_ADDRESS					mGpuVirtualAddress;

	private:
		D_CORE::Ref<D3D12Heap>						mBackingHeap;
		D3D12_HEAP_PROPERTIES						mHeapProperties;

		D3DX12Residency::ManagedObject* mResidencyHandle;

		Microsoft::WRL::ComPtr<ID3D12Resource>		mResource;
		D3D12ResourceDesc							mDesc;

		Microsoft::WRL::ComPtr<ID3D12Device>		mParentDevice;

		uint32_t									mVersionID = 0;
		uint32_t									mRequiresResidencyTracking : 1 = false;
	};

	
}