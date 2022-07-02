#include "./Renderer/pch.hpp"
#include "D3DUtils.hpp"

using namespace Microsoft::WRL;

namespace Darius::Renderer::GraphicsUtils
{
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, const void* initData, UINT64 byteSize, Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer)
	{
		ComPtr<ID3D12Resource> defaultBuffer;

		// Create the acutal default buffer resource.
		auto tmpBuffer = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
		auto heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		D_HR_CHECK(device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&tmpBuffer,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(defaultBuffer.GetAddressOf())
		));

		// In order to copy CPU memory data into our default buffer,
		// to create an intermediate upload heap.
		tmpBuffer = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
		heapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		D_HR_CHECK(device->CreateCommittedResource(
			&heapProp,
			D3D12_HEAP_FLAG_NONE,
			&tmpBuffer,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		));

		// Describe the data we want to copy into the default buffer.
		D3D12_SUBRESOURCE_DATA subResourceData = {};
		subResourceData.pData = initData;
		subResourceData.RowPitch = byteSize;
		subResourceData.SlicePitch = subResourceData.RowPitch;

		// Schedule to copy the data to the default buffer resource.
		// At a high level, the helper function UpdateSubresources
		// will copy the CPU memory into the intermediate upload heap.
		// Then, using ID3D12CommandList::CopySubresourceRegion,
		// the intermediate upload heap data will be copied to mBuffer.
		auto transition = CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST
		);
		cmdList->ResourceBarrier(1, &transition);

		UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);

		transition = CD3DX12_RESOURCE_BARRIER::Transition(
			defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ
		);
		cmdList->ResourceBarrier(1, &transition);

		// Note: uploadBuffer has to be kept alive after the above function
		// calls because the command list has not been executed yet that
		// performs the actual copy.
		// The caller can Release the uploadBuffer after it knows the copy 
		// has been executed.
		return defaultBuffer;
	}

}