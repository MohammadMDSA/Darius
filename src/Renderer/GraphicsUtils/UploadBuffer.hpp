#pragma once

using namespace Microsoft::WRL;

namespace Darius::Renderer::GraphicsUtils
{

	template<typename T>
	class UploadBuffer
	{
	public:
		UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
			mIsConstantBuffer(isConstantBuffer)
		{
			mElementByteSize = sizeof(T);

			// Constant buffer elements need to be multiples of 256 bytes.
			// This is because the hardware can only view constant data
			// at m*256 byte offsets and of n*256 byte lengths.
			// typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
			// UINT64 OffsetInBytes; // multiple of 256
			// UINT SizeInBytes; // multiple of 256
			// } D3D12_CONSTANT_BUFFER_VIEW_DESC;
			if (isConstantBuffer)
				mElementByteSize = CalcConstantBufferByteSize(sizeof(T));
			
			CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
			CD3DX12_RESOURCE_DESC bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize * elementCount);
			D_HR_CHECK(device->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&bufferDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&mUploadBuffer)
			));

			D_HR_CHECK(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));

			// We do not need to unmap until we are done with the resource.
			// However, we muust not write to the resource while it is in use by
			// the GPU (so we must use synchronization techniques).
		}

		UploadBuffer(const UploadBuffer& other) = delete;
		UploadBuffer& operator=(const UploadBuffer& other) = delete;
		~UploadBuffer()
		{
			if (mUploadBuffer != nullptr)
				mUploadBuffer->Unmap(0, nullptr);

			mMappedData = nullptr;
		}

		inline ID3D12Resource* Resource() const
		{
			return mUploadBuffer.Get();
		}

		void CopyData(int elementIndex, const T& data)
		{
			memcpy(&mMappedData[elementIndex * mElementByteSize], &data, sizeof(T));
		}

	private:
		ComPtr<ID3D12Resource>		mUploadBuffer;
		BYTE*						mMappedData = nullptr;
		UINT						mElementByteSize = 0;
		bool						mIsConstantBuffer = false;
	};
}