#include "Renderer/pch.hpp"
#include "AccelerationStructure.hpp"

#include "Renderer/RayTracing/RayTracingCommandContext.hpp"

#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>

using namespace D_GRAPHICS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_PROFILING;
using namespace D_RENDERER_RT_UTILS;

namespace Darius::Renderer::RayTracing::Utils
{
    void AccelerationStructure::CreateDerivedViews()
    {
        // Allocate resource for acceleration structures.
        // Acceleration structures can only be placed in resources that are created in the default heap (or custom heap equivalent). 
        // Default heap is OK since the application doesn’t need CPU read/write access to them. 
        // The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, 
        // and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS. The ALLOW_UNORDERED_ACCESS requirement simply acknowledges both: 
        //  - the system will be doing this type of access in its implementation of acceleration structure builds behind the scenes.
        //  - from the app point of view, synchronization of writes/reads to acceleration structures is accomplished using UAV barriers.
        // Buffer resources must have 64KB alignment which satisfies the AS resource requirement to have alignment of 256 (D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BYTE_ALIGNMENT).

        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
        UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
        UAVDesc.Buffer.CounterOffsetInBytes = 0;
        UAVDesc.Buffer.NumElements = mElementCount;
        UAVDesc.Buffer.StructureByteStride = mElementSize;
        UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

        if (mUAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
            mUAV = D_GRAPHICS::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        D_GRAPHICS_DEVICE::GetDevice()->CreateUnorderedAccessView(mResource.Get(), nullptr, &UAVDesc, mUAV);
    }

    void AccelerationStructure::CreateAccelerationStructure()
    {

        Create(mName, 1, mPrebuildInfo.ResultDataMaxSizeInBytes, nullptr, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
    }

    void BottomLevelAccelerationStructure::UpdateGeometryDescsTransform(D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress)
    {
        struct alignas(16) AlignedGeometryTransform3x4
        {
            float transform3x4[12];
        };

        for (UINT i = 0; i < mGeometryDescs.size(); i++)
        {
            auto& geometryDesc = mGeometryDescs[i];
            geometryDesc.Triangles.Transform3x4 = baseGeometryTransformGPUAddress + i * sizeof(AlignedGeometryTransform3x4);
        }
    }

    // Build geometry descs for bottom-level AS.
    void BottomLevelAccelerationStructure::BuildGeometryDescs(BottomLevelAccelerationStructureGeometry const& bottomLevelASGeometry)
    {
        D3D12_RAYTRACING_GEOMETRY_DESC geometryDescTemplate = {};
        geometryDescTemplate.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
        geometryDescTemplate.Triangles.IndexFormat = bottomLevelASGeometry.m_indexFormat;
        geometryDescTemplate.Triangles.VertexFormat = bottomLevelASGeometry.m_vertexFormat;
        mGeometryDescs.reserve(bottomLevelASGeometry.m_geometryInstances.size());

        for (auto& geometry : bottomLevelASGeometry.m_geometryInstances)
        {
            auto& geometryDesc = geometryDescTemplate;
            geometryDescTemplate.Flags = geometry.geometryFlags;
            geometryDesc.Triangles.IndexBuffer = geometry.ib.indexBuffer;
            geometryDesc.Triangles.IndexCount = geometry.ib.count;
            geometryDesc.Triangles.VertexBuffer = geometry.vb.vertexBuffer;
            geometryDesc.Triangles.VertexCount = geometry.vb.count;
            geometryDesc.Triangles.Transform3x4 = geometry.transform;

            mGeometryDescs.push_back(geometryDesc);
        }
    }

    void BottomLevelAccelerationStructure::ComputePrebuildInfo(ID3D12Device5* device)
    {
        // Get the size requirements for the scratch and AS buffers.
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
        bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
        bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        bottomLevelInputs.Flags = mBuildFlags;
        bottomLevelInputs.NumDescs = static_cast<UINT>(mGeometryDescs.size());
        bottomLevelInputs.pGeometryDescs = mGeometryDescs.data();

        device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &mPrebuildInfo);
        D_HR_CHECK(mPrebuildInfo.ResultDataMaxSizeInBytes > 0);
    }

    void BottomLevelAccelerationStructure::Initialize(
        ID3D12Device5* device,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
        BottomLevelAccelerationStructureGeometry const& bottomLevelASGeometry,
        bool allowUpdate,
        bool bUpdateOnBuild)
    {
        mAllowUpdate = allowUpdate;
        mUpdateOnBuild = bUpdateOnBuild;

        mBuildFlags = buildFlags;
        mName = bottomLevelASGeometry.GetName();

        if (allowUpdate)
        {
            mBuildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
        }

        BuildGeometryDescs(bottomLevelASGeometry);
        ComputePrebuildInfo(device);
        CreateAccelerationStructure();

        mIsDirty = true;
        mIsBuilt = false;
    }

    // The caller must add a UAV barrier before using the resource.
    void BottomLevelAccelerationStructure::Build(
        RayTracingCommandContext& commandList,
        D_GRAPHICS_BUFFERS::GpuBuffer const& scratch,
        D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGPUAddress)
    {
        D_HR_FORCE(mPrebuildInfo.ScratchDataSizeInBytes <= scratch.GetBufferSize(), L"Insufficient scratch buffer size provided!");

        if (baseGeometryTransformGPUAddress > 0)
        {
            UpdateGeometryDescsTransform(baseGeometryTransformGPUAddress);
        }

        currentID = (currentID + 1) % D_GRAPHICS_DEVICE::gNumFrameResources;
        mCacheGeometryDescs[currentID].clear();
        mCacheGeometryDescs[currentID].resize(mGeometryDescs.size());
        copy(mGeometryDescs.begin(), mGeometryDescs.end(), mCacheGeometryDescs[currentID].begin());

        commandList.BuildRaytracingBottomLevelAccelerationStructure(*this, scratch, mCacheGeometryDescs[currentID], mBuildFlags, mIsBuilt && mAllowUpdate && mUpdateOnBuild);

        mIsDirty = false;
        mIsBuilt = true;
    }

    void TopLevelAccelerationStructure::ComputePrebuildInfo(ID3D12Device5* device, UINT numBottomLevelASInstanceDescs)
    {
        // Get the size requirements for the scratch and AS buffers.
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
        topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        topLevelInputs.Flags = mBuildFlags;
        topLevelInputs.NumDescs = numBottomLevelASInstanceDescs;

        device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &mPrebuildInfo);
        D_HR_CHECK(mPrebuildInfo.ResultDataMaxSizeInBytes > 0);
    }

    void TopLevelAccelerationStructure::Initialize(
        ID3D12Device5* device,
        UINT numBottomLevelASInstanceDescs,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
        bool allowUpdate,
        bool bUpdateOnBuild,
        const wchar_t* resourceName)
    {
        mAllowUpdate = allowUpdate;
        mUpdateOnBuild = bUpdateOnBuild;
        mBuildFlags = buildFlags;
        mName = resourceName;

        if (allowUpdate)
        {
            mBuildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
        }

        ComputePrebuildInfo(device, numBottomLevelASInstanceDescs);
        CreateAccelerationStructure();

        mIsDirty = true;
        mIsBuilt = false;
    }

    void TopLevelAccelerationStructure::Build(Darius::Renderer::RayTracing::RayTracingCommandContext& commandList, UINT numBottomLevelASInstanceDescs, TypedStructuredBuffer<BottomLevelAccelerationStructureInstanceDesc> const& bottomLevelASnstanceDescs, UINT frameIndex, D_GRAPHICS_BUFFERS::GpuBuffer const& scratch, bool bUpdate)
    {
        commandList.BuildRaytracingTopLevelAccelerationStructure(*this, numBottomLevelASInstanceDescs, scratch, bottomLevelASnstanceDescs.GetGpuVirtualAddress(frameIndex), mBuildFlags, mIsBuilt && mAllowUpdate && mUpdateOnBuild && bUpdate);
        mIsDirty = false;
        mIsBuilt = true;
    }

    RaytracingAccelerationStructureManager::RaytracingAccelerationStructureManager(ID3D12Device5* device, UINT numBottomLevelInstances, UINT frameCount)
    {
        mBottomLevelASInstanceDescs.Create(device, numBottomLevelInstances, frameCount, L"Bottom-Level Acceleration Structure Instance descs.");
    }

    // Adds a bottom-level Acceleration Structure.
    // The passed in bottom-level AS geometry must have a unique name.
    // Requires a corresponding 1 or more AddBottomLevelASInstance() calls to be added to the top-level AS for the bottom-level AS to be included.
    void RaytracingAccelerationStructureManager::AddBottomLevelAS(
        ID3D12Device5* device,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
        BottomLevelAccelerationStructureGeometry& bottomLevelASGeometry,
        bool allowUpdate,
        bool performUpdateOnBuild)
    {
        D_HR_FORCE(mVBottomLevelAS.find(bottomLevelASGeometry.GetName()) == mVBottomLevelAS.end(),
            L"A bottom level acceleration structure with that name already exists.");

        auto& bottomLevelAS = mVBottomLevelAS[bottomLevelASGeometry.GetName()];

        bottomLevelAS.Initialize(device, buildFlags, bottomLevelASGeometry, allowUpdate);

        mASmemoryFootprint += bottomLevelAS.RequiredResultDataSizeInBytes();
        mScratchResourceSize = std::max(bottomLevelAS.RequiredScratchSize(), mScratchResourceSize);

        mVBottomLevelAS[bottomLevelAS.GetName()] = bottomLevelAS;
    }

    // Adds an instance of a bottom-level Acceleration Structure.
    // Requires a call InitializeTopLevelAS() call to be added to top-level AS.
    UINT RaytracingAccelerationStructureManager::AddBottomLevelASInstance(
        const std::wstring& bottomLevelASname,
        UINT instanceContributionToHitGroupIndex,
        D_MATH::Matrix4 const& transform,
        BYTE instanceMask)
    {
        D_HR_FORCE(mNumBottomLevelASInstances < mBottomLevelASInstanceDescs.NumElements(), L"Not enough instance desc buffer size.");

        UINT instanceIndex = mNumBottomLevelASInstances++;
        auto& bottomLevelAS = mVBottomLevelAS[bottomLevelASname];

        auto& instanceDesc = mBottomLevelASInstanceDescs[instanceIndex];
        instanceDesc.InstanceMask = instanceMask;
        instanceDesc.InstanceContributionToHitGroupIndex = instanceContributionToHitGroupIndex != UINT_MAX ? instanceContributionToHitGroupIndex : bottomLevelAS.GetInstanceContributionToHitGroupIndex();
        instanceDesc.AccelerationStructure = bottomLevelAS.GetResource()->GetGPUVirtualAddress();
        XMStoreFloat3x4(reinterpret_cast<DirectX::XMFLOAT3X4*>(instanceDesc.Transform), transform);

        return instanceIndex;
    };

    UINT RaytracingAccelerationStructureManager::GetMaxInstanceContributionToHitGroupIndex() const
    {
        UINT maxInstanceContributionToHitGroupIndex = 0;
        for (UINT i = 0; i < mNumBottomLevelASInstances; i++)
        {
            auto& instanceDesc = mBottomLevelASInstanceDescs[i];
            maxInstanceContributionToHitGroupIndex = std::max(maxInstanceContributionToHitGroupIndex, instanceDesc.InstanceContributionToHitGroupIndex);
        }
        return maxInstanceContributionToHitGroupIndex;
    };

    // Initializes the top-level Acceleration Structure.
    void RaytracingAccelerationStructureManager::InitializeTopLevelAS(
        ID3D12Device5* device,
        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
        bool allowUpdate,
        bool performUpdateOnBuild,
        const wchar_t* resourceName)
    {
        mTopLevelAS.Initialize(device, GetNumberOfBottomLevelASInstances(), buildFlags, allowUpdate, performUpdateOnBuild, resourceName);

        mASmemoryFootprint += mTopLevelAS.RequiredResultDataSizeInBytes();
        mScratchResourceSize = std::max(mTopLevelAS.RequiredScratchSize(), mScratchResourceSize);

        mAccelerationStructureScratch.Create(L"Acceleration structure scratch resource", 1, mScratchResourceSize);
    }

    // Builds all bottom-level and top-level Acceleration Structures.
    void RaytracingAccelerationStructureManager::Build(
        RayTracingCommandContext& commandList,
        ID3D12DescriptorHeap* descriptorHeap,
        UINT frameIndex,
        bool bForceBuild)
    {
        ScopedTimer _prof(L"Acceleration Structure build", commandList);

        mBottomLevelASInstanceDescs.CopyStagingToGpu(frameIndex);

        // Build all bottom-level AS.
        {
            ScopedTimer _prof(L"Bottom Level AS", commandList);
            for (auto& bottomLevelASpair : mVBottomLevelAS)
            {
                auto& bottomLevelAS = bottomLevelASpair.second;
                if (bForceBuild || bottomLevelAS.IsDirty())
                {
                    ScopedTimer _prof(bottomLevelAS.GetName(), commandList);

                    D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;
                    bottomLevelAS.Build(commandList, mAccelerationStructureScratch, baseGeometryTransformGpuAddress);

                    // Since a single scratch resource is reused, put a barrier in-between each call.
                    // PEFORMANCE tip: use separate scratch memory per BLAS build to allow a GPU driver to overlap build calls.
                    commandList.TransitionResource(bottomLevelAS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
                }
            }
        }

        // Build the top-level AS.
        {
            ScopedTimer _prof(L"Top Level AS", commandList);

            bool performUpdate = false; // Always rebuild top-level Acceleration Structure.
            mTopLevelAS.Build(commandList, GetNumberOfBottomLevelASInstances(), mBottomLevelASInstanceDescs, frameIndex, mAccelerationStructureScratch, performUpdate);

            commandList.TransitionResource(mTopLevelAS, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, true);
        }
    }

    void BottomLevelAccelerationStructureInstanceDesc::SetTransform(D_MATH::Matrix4 const& transform)
    {
        XMStoreFloat3x4(reinterpret_cast<DirectX::XMFLOAT3X4*>(Transform), transform);
    }

    void BottomLevelAccelerationStructureInstanceDesc::GetTransform(D_MATH::Matrix4& transform) const
    {
        transform = D_MATH::Matrix4(XMLoadFloat3x4(reinterpret_cast<DirectX::XMFLOAT3X4 const*>(Transform)));
    }
}