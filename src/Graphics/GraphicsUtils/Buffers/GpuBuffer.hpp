#pragma once

#include "Graphics/GraphicsUtils/GpuResource.hpp"
#include "Graphics/GraphicsUtils/Buffers/UploadBuffer.hpp"

#ifndef D_GRAPHICS_BUFFERS
#define D_GRAPHICS_BUFFERS Darius::Graphics::Utils::Buffers
#endif

namespace Darius::Graphics
{
    class CommandContext;
}

namespace Darius::Graphics::Utils::Buffers
{
    class UploadBuffer;

    class GpuBuffer : public GpuResource
    {
    public:
        virtual ~GpuBuffer() { Destroy(); }

        // Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
        void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const void* initialData = nullptr);

        void Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize, const UploadBuffer& srcData, uint32_t srcOffset = 0);

        // Sub-Allocate a buffer out of a pre-allocated heap.  If initial data is provided, it will be copied into the buffer using the default command context.
        void CreatePlaced(const std::wstring& name, ID3D12Heap* pBackingHeap, uint32_t HeapOffset, uint32_t NumElements, uint32_t ElementSize,
            const void* initialData = nullptr);

        const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return mUAV; }
        const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return mSRV; }

        D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView(void) const { return mGpuVirtualAddress; }

        D3D12_CPU_DESCRIPTOR_HANDLE CreateConstantBufferView(uint32_t Offset, uint32_t Size) const;

        D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const;
        D3D12_VERTEX_BUFFER_VIEW VertexBufferView(size_t BaseVertexIndex = 0) const
        {
            size_t Offset = BaseVertexIndex * mElementSize;
            return VertexBufferView(Offset, (uint32_t)(mBufferSize - Offset), mElementSize);
        }

        D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t Offset, uint32_t Size, bool b32Bit = false) const;
        D3D12_INDEX_BUFFER_VIEW IndexBufferView(size_t StartIndex = 0) const
        {
            size_t Offset = StartIndex * mElementSize;
            return IndexBufferView(Offset, (uint32_t)(mBufferSize - Offset), mElementSize == 4);
        }

        size_t GetBufferSize() const { return mBufferSize; }
        uint32_t GetElementCount() const { return mElementCount; }
        uint32_t GetElementSize() const { return mElementSize; }

    protected:
        GpuBuffer(void) : mBufferSize(0), mElementCount(0), mElementSize(0)
        {
            mResourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            mUAV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
            mSRV.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
        }

        D3D12_RESOURCE_DESC DescribeBuffer(void);
        virtual void CreateDerivedViews(void) = 0;

        D3D12_CPU_DESCRIPTOR_HANDLE mUAV;
        D3D12_CPU_DESCRIPTOR_HANDLE mSRV;

        size_t mBufferSize;
        uint32_t mElementCount;
        uint32_t mElementSize;
        D3D12_RESOURCE_FLAGS mResourceFlags;
    };

    inline D3D12_VERTEX_BUFFER_VIEW GpuBuffer::VertexBufferView(size_t Offset, uint32_t Size, uint32_t Stride) const
    {
        D3D12_VERTEX_BUFFER_VIEW VBView;
        VBView.BufferLocation = mGpuVirtualAddress + Offset;
        VBView.SizeInBytes = Size;
        VBView.StrideInBytes = Stride;
        return VBView;
    }

    inline D3D12_INDEX_BUFFER_VIEW GpuBuffer::IndexBufferView(size_t Offset, uint32_t Size, bool b32Bit) const
    {
        D3D12_INDEX_BUFFER_VIEW IBView;
        IBView.BufferLocation = mGpuVirtualAddress + Offset;
        IBView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
        IBView.SizeInBytes = Size;
        return IBView;
    }

    class ByteAddressBuffer : public GpuBuffer
    {
    public:
        virtual void CreateDerivedViews(void) override;
    };

    class IndirectArgsBuffer : public ByteAddressBuffer
    {
    public:
        IndirectArgsBuffer(void)
        {
        }
    };

    class StructuredBuffer : public GpuBuffer
    {
    public:
        virtual void Destroy(void) override
        {
            mCounterBuffer.Destroy();
            GpuBuffer::Destroy();
        }

        virtual void CreateDerivedViews(void) override;

        ByteAddressBuffer& GetCounterBuffer(void) { return mCounterBuffer; }

        const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterSRV(CommandContext& Context);
        const D3D12_CPU_DESCRIPTOR_HANDLE& GetCounterUAV(CommandContext& Context);

    private:
        ByteAddressBuffer mCounterBuffer;
    };

    class TypedBuffer : public GpuBuffer
    {
    public:
        TypedBuffer(DXGI_FORMAT Format) : mDataFormat(Format) {}
        virtual void CreateDerivedViews(void) override;

    protected:
        DXGI_FORMAT mDataFormat;
    };

}