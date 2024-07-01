#include "Renderer/pch.hpp"
#include "AccelerationStructure.hpp"

#include "Renderer/RayTracing/RayTracingCommandContext.hpp"
#include "Renderer/RayTracing/Renderer.hpp"

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

		auto device = D_GRAPHICS_DEVICE::GetDevice();

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
		UAVDesc.Buffer.CounterOffsetInBytes = 0;
		UAVDesc.Buffer.NumElements = mElementCount;
		UAVDesc.Buffer.StructureByteStride = mElementSize;
		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		if (mUAV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mUAV = D_GRAPHICS::AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateUnorderedAccessView(GetResource(), nullptr, &UAVDesc, mUAV);

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.RaytracingAccelerationStructure.Location = GetGpuVirtualAddress();

		if (mSRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
			mSRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateShaderResourceView(nullptr, &SRVDesc, mSRV);
	}

	void AccelerationStructure::CreateAccelerationStructure()
	{

		Create(mName, 1, (UINT32)mPrebuildInfo.ResultDataMaxSizeInBytes, nullptr, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE);
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
		geometryDescTemplate.Triangles.IndexFormat = D_RENDERER_GEOMETRY::Mesh::StandardIndexFormat;
		geometryDescTemplate.Triangles.VertexFormat = D_RENDERER_GEOMETRY::Mesh::StandardVertexFormat;

		auto const& mesh = bottomLevelASGeometry.Mesh;
		mGeometryDescs.reserve(bottomLevelASGeometry.Mesh.mDraw.size());
		D_ASSERT(mesh.mDraw.size() == mesh.IndexDataGpu.size());

		for (int i = 0; i < mesh.mDraw.size(); i++)
		{
			GeometryVertexIndexViews gviv;
			D3D12_RAYTRACING_GEOMETRY_DESC geomDesc = geometryDescTemplate;
			geomDesc.Flags = bottomLevelASGeometry.Flags;
			geomDesc.Triangles.IndexBuffer = bottomLevelASGeometry.Mesh.IndexDataGpu[i].GetGpuVirtualAddress();
			geomDesc.Triangles.IndexCount = bottomLevelASGeometry.Mesh.mDraw[i].IndexCount;
			geomDesc.Triangles.VertexBuffer = bottomLevelASGeometry.Mesh.VertexDataGpu.GetGpuVirtualAddressAndStride();
			geomDesc.Triangles.VertexCount = bottomLevelASGeometry.Mesh.mNumTotalVertices;

			// Allocating SRV for vertex and index buffer
			gviv = { D_RENDERER_RT::AllocateTextureDescriptor(2) };

			UINT srcCount[] = { 1u, 1u };
			UINT destCount = 2u;

			D3D12_CPU_DESCRIPTOR_HANDLE srcDescriptors[] = { bottomLevelASGeometry.Mesh.IndexDataGpu[i].GetSRV(), bottomLevelASGeometry.Mesh.VertexDataGpu.GetSRV() };

			D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptors(1, &gviv.IndexVertexBufferSRV, &destCount, 2, srcDescriptors, srcCount, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			mGeometryDescs.push_back(geomDesc);
			mGeometryMeshViews.push_back(gviv);
		}
	}

	void BottomLevelAccelerationStructure::ComputePrebuildInfo()
	{
		// Get the size requirements for the scratch and AS buffers.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
		bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		bottomLevelInputs.Flags = mBuildFlags;
		bottomLevelInputs.NumDescs = static_cast<UINT>(mGeometryDescs.size());
		bottomLevelInputs.pGeometryDescs = mGeometryDescs.data();

		auto device = D_GRAPHICS_DEVICE::GetDevice5();

		device->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &mPrebuildInfo);
		D_HR_CHECK(mPrebuildInfo.ResultDataMaxSizeInBytes > 0);
	}

	void BottomLevelAccelerationStructure::Initialize(
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags,
		BottomLevelAccelerationStructureGeometry const& bottomLevelASGeometry,
		bool allowUpdate,
		bool bUpdateOnBuild)
	{
		mAllowUpdate = allowUpdate;
		mUpdateOnBuild = bUpdateOnBuild;

		mBuildFlags = buildFlags;
		mUuid = bottomLevelASGeometry.Uuid;

		if (allowUpdate)
		{
			mBuildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		}

		BuildGeometryDescs(bottomLevelASGeometry);
		ComputePrebuildInfo();
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

	void TopLevelAccelerationStructure::ComputePrebuildInfo(UINT numBottomLevelASInstanceDescs)
	{
		// Get the size requirements for the scratch and AS buffers.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
		topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
		topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		topLevelInputs.Flags = mBuildFlags;
		topLevelInputs.NumDescs = numBottomLevelASInstanceDescs;

		auto device = D_GRAPHICS_DEVICE::GetDevice5();
		device->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &mPrebuildInfo);
		D_HR_CHECK(mPrebuildInfo.ResultDataMaxSizeInBytes > 0);
	}

	void TopLevelAccelerationStructure::Initialize(
		UINT maxNumBottomLevelASInstanceDescs,
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

		ComputePrebuildInfo(maxNumBottomLevelASInstanceDescs);
		CreateAccelerationStructure();

		mIsDirty = true;
		mIsBuilt = false;
	}

	void TopLevelAccelerationStructure::Build(Darius::Renderer::RayTracing::RayTracingCommandContext& commandList, UINT numBottomLevelASInstanceDescs, D_GRAPHICS_BUFFERS::StructuredUploadBuffer <BottomLevelAccelerationStructureInstanceDesc> const& bottomLevelASnstanceDescs, UINT frameIndex, D_GRAPHICS_BUFFERS::GpuBuffer const& scratch, bool bUpdate)
	{
		commandList.BuildRaytracingTopLevelAccelerationStructure(*this, numBottomLevelASInstanceDescs, scratch, bottomLevelASnstanceDescs.GetGpuVirtualAddress(frameIndex), mBuildFlags, mIsBuilt && mAllowUpdate && mUpdateOnBuild && bUpdate);
		mIsDirty = false;
		mIsBuilt = true;
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