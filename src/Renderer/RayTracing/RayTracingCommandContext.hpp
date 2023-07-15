#pragma once

#include "Utils/AccelerationStructure.hpp"

#include <Core/Containers/Vector.hpp>
#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>

#ifndef D_RENDERER_RT
#define D_RENDERER_RT Darius::Renderer::RayTracing
#endif // !D_RENDERER_RT_UTILS

namespace Darius::Renderer::RayTracing
{
	class RayTracingCommandContext : public D_GRAPHICS::CommandContext
	{
	public:

		static RayTracingCommandContext& Begin(std::wstring const& ID = L"")
		{
			auto& context = D_GRAPHICS::CommandContext::Begin(ID);
			return static_cast<RayTracingCommandContext&>(context);
		}

	public:
		void BuildRaytracingTopLevelAccelerationStructure(D_RENDERER_RT_UTILS::TopLevelAccelerationStructure const& tlas, UINT numBottomLevelASInstanceDescs, D_GRAPHICS_BUFFERS::GpuBuffer const& scratch, D3D12_GPU_VIRTUAL_ADDRESS bottomLevelASInstanceDescs, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE, bool update = false);

		void BuildRaytracingBottomLevelAccelerationStructure(D_RENDERER_RT_UTILS::BottomLevelAccelerationStructure const& blas, D_GRAPHICS_BUFFERS::GpuBuffer const& scratch, D_CONTAINERS::DVector<D3D12_RAYTRACING_GEOMETRY_DESC> const& geometries, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS buildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE, bool update = false);

	};
}