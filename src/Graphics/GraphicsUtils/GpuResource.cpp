#include "Graphics/pch.hpp"
#include "GpuResource.hpp"

#include "Graphics/GraphicsDeviceManager.hpp"
#include "Residency.hpp"

namespace Darius::Graphics::Utils
{

	D3D12Heap::D3D12Heap(ID3D12Device* Parent) :
		mParentDevice(Parent),
		mHeapDesc()
	{ }

	void D3D12Heap::Destroy()
	{
		if(mResidencyHandle)
		{
			if(mResidencyHandle->IsInitialized())
				D_GRAPHICS_DEVICE::GetResidencyManager().EndTrackingObject(mResidencyHandle);

			delete mResidencyHandle;
		}
		mHeap.Reset();
	}

	D3D12Heap::~D3D12Heap()
	{
		Destroy();
	}

	void D3D12Heap::SetHeap(ID3D12Heap* HeapIn, std::string const& InName, bool InTrack, bool bForceGetGPUAddress)
	{
		mHeap = HeapIn;
		mTrack = InTrack;
		mHeapDesc = mHeap->GetDesc();

		mRequiresResidencyTracking = IsGPUOnly(mHeapDesc.Properties.Type, &mHeapDesc.Properties);

	}

	void D3D12Heap::DisallowTrackingResidency()
	{
		D_ASSERT_M(mResidencyHandle == nullptr, "Can't disallow residency tracking after it has started. Call this function instead of BeginTrackingResidency().");
		mRequiresResidencyTracking = false;
	}

	void D3D12Heap::BeginTrackingResidency(size_t Size)
	{
		D_ASSERT_M(mRequiresResidencyTracking, "Residency tracking is not expected for this resource");
		D_ASSERT_M(!mResidencyHandle, "Residency tracking is already initialzied for this resource");
		mResidencyHandle = new D3DX12Residency::ManagedObject();
		mResidencyHandle->Initialize(mHeap.Get(), Size);
		D_GRAPHICS_DEVICE::GetResidencyManager().BeginTrackingObject(mResidencyHandle);
	}

	void GpuResource::InitializeInternal()
	{ }

	void GpuResource::Destroy()
	{
		mResource = nullptr;
		mBackingHeap = nullptr;

		if(mResidencyHandle != nullptr)
		{
			if(mResidencyHandle->IsInitialized())
			{
				D_GRAPHICS_DEVICE::GetResidencyManager().EndTrackingObject(mResidencyHandle);
			}

			delete mResidencyHandle;
			mResidencyHandle = nullptr;
		}

		mGpuVirtualAddress = D3D12_GPU_VIRTUAL_ADDRESS_NULL;
		mVersionID++;
	}

	HRESULT GpuResource::CreateCommittedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, D3D12_HEAP_PROPERTIES const& heapProps, D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue)
	{
		D_ASSERT(device);
		D_ASSERT(!mDesc.ReservedResource);

		if(mResource != nullptr)
			Destroy();

		mDesc = desc;
		mParentDevice = device;
		mHeapProperties = heapProps;

		mRequiresResidencyTracking = IsGPUOnly(mHeapProperties.Type, &mHeapProperties);

		if(mRequiresResidencyTracking)
			StartTrackingForResidency();

		return mParentDevice->CreateCommittedResource(&heapProps, heapFlags, &desc, initialState, clearValue, IID_PPV_ARGS(mResource.ReleaseAndGetAddressOf()));
	}

	HRESULT GpuResource::CreatePlacedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, D3D12Heap* heap, uint64_t heapOffset, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue)
	{
		D_ASSERT(device);
		D_ASSERT(heap);
		D_ASSERT(!mDesc.ReservedResource);

		if(mResource != nullptr)
			Destroy();

		mDesc = desc;
		mBackingHeap = heap;
		mParentDevice = device;
		mHeapProperties = heap->GetHeapDesc().Properties;

		mRequiresResidencyTracking = IsGPUOnly(mHeapProperties.Type, &mHeapProperties);

		return mParentDevice->CreatePlacedResource(heap->GetHeap(), heapOffset, &desc, initialState, clearValue, IID_PPV_ARGS(mResource.ReleaseAndGetAddressOf()));
	}

	HRESULT GpuResource::CreateResesrvedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue)
	{
		D_ASSERT_NOENTRY();
		return (HRESULT)0;
		/*D_ASSERT(device);
		D_ASSERT(desc.ReservedResource);

		mDesc = desc;
		mParentDevice = device;
		
		mRequiresResidencyTracking = IsGPUOnly()

		device->CreateReservedResource(&desc, initialState, clearValue, IID_PPV_ARGS(mResource.ReleaseAndGetAddressOf()));*/
	}

	void GpuResource::AttachOther(ID3D12Resource* other)
	{
		mResource.Attach(other);
	}

	void GpuResource::StartTrackingForResidency()
	{
		if(mRequiresResidencyTracking)
			return;

		D_ASSERT_M(IsGPUOnly(mHeapProperties.Type), "Residency tracking is not expected for CPU-accessible resources");
		D_ASSERT_M(!mResidencyHandle, "The previous residency handle has not been uninitialized yet.");

		if(IsPlacedResource() || IsReservedResource())
			return;

		mResidencyHandle = new D3DX12Residency::ManagedObject();
		auto info = GetParentDevice()->GetResourceAllocationInfo(0, 1, &mDesc);
		mResidencyHandle->Initialize(mResource.Get(), info.SizeInBytes);
		D_GRAPHICS_DEVICE::GetResidencyManager().BeginTrackingObject(mResidencyHandle);
	}

}
