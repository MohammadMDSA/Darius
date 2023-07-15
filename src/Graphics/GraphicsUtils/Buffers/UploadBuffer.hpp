#pragma once

#include "Graphics/GraphicsUtils/GpuResource.hpp"

#include <Core/Containers/Vector.hpp>

#ifndef D_GRAPHICS_BUFFERS
#define D_GRAPHICS_BUFFERS Darius::Graphics::Utils::Buffers
#endif

namespace Darius::Graphics::Utils::Buffers
{
    /// <summary>
    /// Upload buffer with supporting multiple instances (frame resources)
    /// </summary>
    class UploadBuffer : public GpuResource
    {
    public:
        INLINE virtual ~UploadBuffer() { Destroy(); }

        void Create(std::wstring const& name, size_t BufferSize, UINT numInstances = 1);

        void* Map(void);
        void Unmap(size_t begin = 0, size_t end = -1);

        INLINE size_t GetBufferSize() const { return mBufferSize; }
        INLINE size_t GetTotalBufferSize() const { return mBufferSize * mNumInstances; }

        INLINE D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(UINT instanceIndex = 0) const { return mGpuVirtualAddress + instanceIndex * mBufferSize; }


    protected:

        size_t mBufferSize;
        UINT mNumInstances;
    };

    template<typename T>
    class StructuredUploadBuffer : public UploadBuffer
    {
    public:
        // Performance tip: Align structures on sizeof(float4) boundary.
        // Ref: https://developer.nvidia.com/content/understanding-structured-buffer-performance
        D_STATIC_ASSERT_M(sizeof(T) % 16 == 0, "Align structure buffers on 16 byte boundary for performance reasons.");

        StructuredUploadBuffer() : mMappedBuffers(nullptr) {}
        virtual ~StructuredUploadBuffer()
        {
            if (mMappedBuffers != nullptr)
                Unmap();
        }

        INLINE void Create(std::wstring const& name, UINT numElements, UINT numInstances = 1)
        {
            mStaging.resize(numElements);
            size_t bufferSize = numElements * sizeof(T);
            UploadBuffer::Create(name, bufferSize, numInstances);
            mMappedBuffers = reinterpret_cast<T*>(Map());
        }

        INLINE void CopyStagingToGpu(UINT instanceIndex = 0)
        {
            memcpy(mMappedBuffers + instanceIndex * GetNumElements(), &mStaging[0], GetBufferSize());
        }

        auto Begin() { return mStaging.begin(); }
        auto End() { return mStaging.end(); }
        auto Begin() const { return mStaging.begin(); }
        auto End() const { return mStaging.end(); }

        INLINE size_t GetNumElements() const { return mStaging.size(); }
        T& operator[](UINT elementIndex) { return mStaging[elementIndex]; }
        T const& operator[](UINT elementIndex) const { return mStaging[elementIndex]; }

    private:
        T*                          mMappedBuffers;
        D_CONTAINERS::DVector<T>    mStaging;
    };
}