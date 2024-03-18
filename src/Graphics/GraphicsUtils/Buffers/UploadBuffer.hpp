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

        UploadBuffer() = default;
        UploadBuffer(UploadBuffer const&) = default;
        INLINE virtual ~UploadBuffer() { Destroy(); }
        UploadBuffer& operator= (UploadBuffer const&) = default;

        void Create(std::wstring const& name, size_t BufferSize, UINT numInstances = 1);

        void* MapInstance(UINT instanceIndex, bool fillZero = false) const;
        void* Map(bool fillZero = false) const;
        void Unmap(size_t begin = 0, size_t end = -1) const;

        INLINE size_t GetBufferSize() const { return mBufferSize; }
        INLINE size_t GetTotalBufferSize() const { return mBufferSize * mNumInstances; }
        INLINE size_t GetInstanceOffset(UINT instance) { return instance * GetBufferSize(); }


        INLINE D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(UINT instanceIndex = 0) const { return mGpuVirtualAddress + instanceIndex * mBufferSize; }


    protected:

        size_t mBufferSize = 0;
        UINT mNumInstances = 0;
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
            // Already initialized and size is fine
            if (mMappedBuffers != nullptr && (UINT)mStaging.size() >= numElements)
                return;

            // It should be created, unmap if already mapped
            if (mMappedBuffers != nullptr)
                Unmap();

            mStaging.resize(numElements);
            size_t bufferSize = numElements * sizeof(T);
            UploadBuffer::Create(name, bufferSize, numInstances);
            mMappedBuffers = reinterpret_cast<T*>(Map());
        }

        INLINE void CopyStagingToGpu(UINT instanceIndex = 0)
        {
            memcpy(mMappedBuffers + instanceIndex * GetNumElements(), &mStaging[0], GetBufferSize());
        }

        auto begin() { return mStaging.begin(); }
        auto end() { return mStaging.end(); }
        auto begin() const { return mStaging.begin(); }
        auto end() const { return mStaging.end(); }

        INLINE size_t GetNumElements() const { return mStaging.size(); }
        T& operator[](UINT elementIndex) { return mStaging[elementIndex]; }
        T const& operator[](UINT elementIndex) const { return mStaging[elementIndex]; }

    private:
        T*                          mMappedBuffers;
        D_CONTAINERS::DVector<T>    mStaging;
    };
}