#include "Graphics/pch.hpp"
#include "GpuResource.hpp"

namespace Darius::Graphics::Utils
{
	HRESULT GpuResource::CreateCommittedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, D3D12_HEAP_PROPERTIES const& heapProps, D3D12_HEAP_FLAGS heapFlags, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue)
	{
		D_ASSERT(device);
		mDesc = desc;
		mParentDevice = device;
		return mParentDevice->CreateCommittedResource(&heapProps, heapFlags, &desc, initialState, clearValue, IID_PPV_ARGS(mResource.ReleaseAndGetAddressOf()));
	}

	HRESULT GpuResource::CreatePlacedResource(ID3D12Device* device, D3D12ResourceDesc const& desc, ID3D12Heap* heap, uint64_t heapOffset, D3D12_RESOURCE_STATES initialState, D3D12_CLEAR_VALUE const* clearValue)
	{
		D_ASSERT(device);
		mDesc = desc;
		mParentDevice = device;
		return mParentDevice->CreatePlacedResource(heap, heapOffset, &desc, initialState, clearValue, IID_PPV_ARGS(mResource.ReleaseAndGetAddressOf()));
	}
	
	void GpuResource::AttachOther(ID3D12Resource* other)
	{
		mResource.Attach(other);
	}

}
