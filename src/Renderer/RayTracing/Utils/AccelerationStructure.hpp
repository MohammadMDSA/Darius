#pragma once

#include "Renderer/Geometry/Mesh.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/Uuid.hpp>
#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/UploadBuffer.hpp>
#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>
#include <Math/VectorMath.hpp>

#ifndef D_RENDERER_RT_UTILS
#define D_RENDERER_RT_UTILS Darius::Renderer::RayTracing::Utils
#endif // !D_RENDERER_RT_UTILS

namespace Darius::Renderer::RayTracing
{
    class RayTracingCommandContext;
}

namespace Darius::Renderer::RayTracing::Utils
{

    struct AccelerationStructureBuffers
    {
        Microsoft::WRL::ComPtr<ID3D12Resource> scratch;
        Microsoft::WRL::ComPtr<ID3D12Resource> accelerationStructure;
        Microsoft::WRL::ComPtr<ID3D12Resource> instanceDesc;    // Used only for top-level AS
        UINT64                 ResultDataMaxSizeInBytes;
    };

    // AccelerationStructure
    // A base class for bottom-level and top-level AS.
    class AccelerationStructure : public D_GRAPHICS_BUFFERS::GpuBuffer
    {
    public:
        AccelerationStructure() {}
        INLINE UINT64                   RequiredScratchSize() const { return std::max(mPrebuildInfo.ScratchDataSizeInBytes, mPrebuildInfo.UpdateScratchDataSizeInBytes); }
        INLINE UINT64                   RequiredResultDataSizeInBytes() const { return mPrebuildInfo.ResultDataMaxSizeInBytes; }
        INLINE D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO const& PrebuildInfo() const { return mPrebuildInfo; }
        INLINE D_CORE::Uuid const&      GetUuid() const { return mUuid; }
        INLINE std::wstring const&      GetName() const { return mName; }

        INLINE void                     SetDirty(bool isDirty = true) { mIsDirty = isDirty; }
        INLINE bool                     IsDirty() const { return mIsDirty; }

        virtual void                    CreateDerivedViews() override;

        // This method must be called affter after fetching prebuild info
        void                            CreateAccelerationStructure();

    protected:
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS mBuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO mPrebuildInfo = {};
        D_CORE::Uuid            mUuid;
        std::wstring            mName;

        bool                    mIsBuilt = false; // whether the AS has been built at least once.
        bool                    mIsDirty = true; // whether the AS has been modified and needs to be rebuilt.
        bool                    mUpdateOnBuild = false;
        bool                    mAllowUpdate = false;
        
    };

    struct BottomLevelAccelerationStructureGeometry
    {
        D_RENDERER_GEOMETRY::Mesh const&    Mesh;

        D_CORE::Uuid const                  Uuid;
        D3D12_RAYTRACING_GEOMETRY_FLAGS     Flags;
    };

    struct GeometryVertexIndexViews
    {
        D_GRAPHICS_MEMORY::DescriptorHandle IndexVertexBufferSRV;
    };

    struct BottomLevelAccelerationStructureInstanceDesc : public D3D12_RAYTRACING_INSTANCE_DESC
    {
        void                            SetTransform(D_MATH::Matrix4 const& transform);
        void                            GetTransform(D_MATH::Matrix4& transform) const;
    };
    static_assert(sizeof(BottomLevelAccelerationStructureInstanceDesc) == sizeof(D3D12_RAYTRACING_INSTANCE_DESC), L"This is a wrapper used in place of the desc. It has to have the same size");

    class BottomLevelAccelerationStructure : public AccelerationStructure
    {
    public:
        BottomLevelAccelerationStructure() {};
        ~BottomLevelAccelerationStructure() {}

        void                            Initialize(D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, BottomLevelAccelerationStructureGeometry const& bottomLevelASGeometry, bool allowUpdate = false, bool bUpdateOnBuild = false);
        void                            Build(Darius::Renderer::RayTracing::RayTracingCommandContext& commandList, D_GRAPHICS_BUFFERS::GpuBuffer const& scratch, D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress = 0);

        void                            UpdateGeometryDescsTransform(D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress);

        INLINE UINT                     GetInstanceContributionToHitGroupIndex() const { return mInstanceContributionToHitGroupIndex; }
        INLINE void                     SetInstanceContributionToHitGroupIndex(UINT index) { mInstanceContributionToHitGroupIndex = index; }

        INLINE D_MATH::Matrix4 const&   GetTransform() const { return mTransform; }
        INLINE std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> const& GetGeometryDescs() const { return mGeometryDescs; }
        INLINE UINT                     GetNumGeometries() const { return (UINT)mGeometryDescs.size(); }
        INLINE std::vector<GeometryVertexIndexViews> const& GetGeometryMeshViews() const { return mGeometryMeshViews; }
        INLINE GeometryVertexIndexViews const& GetGeometryMeshViewsByIndex(UINT index) const { return mGeometryMeshViews[index]; }

    private:
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> mGeometryDescs;
        std::vector<GeometryVertexIndexViews>       mGeometryMeshViews;
        UINT                                        currentID = 0;
        std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> mCacheGeometryDescs[D_GRAPHICS_DEVICE::gNumFrameResources];
        D_MATH::Matrix4                             mTransform;
        UINT                                        mInstanceContributionToHitGroupIndex = 0;

        void                            BuildGeometryDescs(BottomLevelAccelerationStructureGeometry const& bottomLevelASGeometry);
        void                            ComputePrebuildInfo();
    };

    class TopLevelAccelerationStructure : public AccelerationStructure
    {
    public:
        TopLevelAccelerationStructure() {}
        ~TopLevelAccelerationStructure() {}

        // Initialization must be performed every time the max number of BLASes is changed
        void                            Initialize(UINT maxNumBottomLevelASInstanceDescs, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, bool allowUpdate = false, bool bUpdateOnBuild = false, const wchar_t* resourceName = nullptr);
        void                            Build(Darius::Renderer::RayTracing::RayTracingCommandContext& commandList, UINT numBottomLevelASInstanceDescs, D_GRAPHICS_BUFFERS::StructuredUploadBuffer<BottomLevelAccelerationStructureInstanceDesc> const& InstanceDescs, UINT frameIndex, D_GRAPHICS_BUFFERS::GpuBuffer const& scratch, bool bUpdate = false);

    private:
        void                            ComputePrebuildInfo(UINT numBottomLevelASInstanceDescs);
    };

}
