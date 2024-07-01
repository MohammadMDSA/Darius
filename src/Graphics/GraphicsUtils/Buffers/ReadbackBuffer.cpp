#include "Graphics/pch.hpp"
#include "ReadbackBuffer.hpp"

#include "PixelBuffer.hpp"
#include "Graphics/GraphicsCore.hpp"
#include "Graphics/GraphicsDeviceManager.hpp"

namespace Darius::Graphics::Utils::Buffers
{
    void ReadbackBuffer::Create(const std::wstring& name, uint32_t NumElements, uint32_t ElementSize)
    {
        Destroy();

        mElementCount = NumElements;
        mElementSize = ElementSize;
        mBufferSize = NumElements * ElementSize;
        mUsageState = D3D12_RESOURCE_STATE_COPY_DEST;

        // Create a readback buffer large enough to hold all texel data
        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.Type = D3D12_HEAP_TYPE_READBACK;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;

        // Readback buffers must be 1-dimensional, i.e. "buffer" not "texture2d"
        D3D12ResourceDesc ResourceDesc = {};
        ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        ResourceDesc.Width = mBufferSize;
        ResourceDesc.Height = 1;
        ResourceDesc.DepthOrArraySize = 1;
        ResourceDesc.MipLevels = 1;
        ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        ResourceDesc.SampleDesc.Count = 1;
        ResourceDesc.SampleDesc.Quality = 0;
        ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        D_HR_CHECK(CreateCommittedResource(
            D_GRAPHICS_DEVICE::GetDevice(),
            ResourceDesc,
            HeapProps,
            D3D12_HEAP_FLAG_NONE,
            D3D12_RESOURCE_STATE_COPY_DEST,
            nullptr));

        mGpuVirtualAddress = GetResource()->GetGPUVirtualAddress();

#ifdef RELEASE
        (name);
#else
        GetResource()->SetName(name.c_str());
#endif
    }

    void* ReadbackBuffer::Map(void)
    {
        void* Memory;
        auto range = CD3DX12_RANGE(0, mBufferSize);
        GetResource()->Map(0, &range, &Memory);
        return Memory;
    }

    void ReadbackBuffer::Unmap(void)
    {
        auto range = CD3DX12_RANGE(0, 0);
        GetResource()->Unmap(0, &range);
    }
}