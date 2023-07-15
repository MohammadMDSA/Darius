#include "Renderer/pch.hpp"

#include "RayTracingCommandContext.hpp"

namespace Darius::Renderer::RayTracing
{
	void RayTracingCommandContext::BuildRaytracingTopLevelAccelerationStructure(D_RENDERER_RT_UTILS::TopLevelAccelerationStructure const& tlas, UINT numBottomLevelASInstanceDescs, D_GRAPHICS_BUFFERS::GpuBuffer const& scratch, D3D12_GPU_VIRTUAL_ADDRESS bottomLevelASInstanceDescs, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, bool update)
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& topLevelInputs = topLevelBuildDesc.Inputs;
		{
			topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
			topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			topLevelInputs.Flags = buildFlags;
			if (update)
			{
				topLevelInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
				topLevelBuildDesc.SourceAccelerationStructureData = tlas.GetGpuVirtualAddress();
			}
			topLevelInputs.NumDescs = numBottomLevelASInstanceDescs;

			topLevelBuildDesc.ScratchAccelerationStructureData = scratch.GetGpuVirtualAddress();
			topLevelBuildDesc.DestAccelerationStructureData = tlas.GetGpuVirtualAddress();
		}
		topLevelInputs.InstanceDescs = bottomLevelASInstanceDescs;

		reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CommandList)->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, nullptr);
	}

	void RayTracingCommandContext::BuildRaytracingBottomLevelAccelerationStructure(D_RENDERER_RT_UTILS::BottomLevelAccelerationStructure const& blas, D_GRAPHICS_BUFFERS::GpuBuffer const& scratch, D_CONTAINERS::DVector<D3D12_RAYTRACING_GEOMETRY_DESC> const& geometries, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags, bool update)
	{
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
		{
			bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			bottomLevelInputs.Flags = buildFlags;
			if (update)
			{
				bottomLevelInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
				bottomLevelBuildDesc.SourceAccelerationStructureData = blas.GetGpuVirtualAddress();
			}
			bottomLevelInputs.NumDescs = static_cast<UINT>(geometries.size());
			bottomLevelInputs.pGeometryDescs = geometries.data();

			bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch.GetGpuVirtualAddress();
			bottomLevelBuildDesc.DestAccelerationStructureData = blas.GetGpuVirtualAddress();
		}

		reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CommandList)->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);
	}
}