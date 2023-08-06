#pragma once

#include "Utils/AccelerationStructure.hpp"

#include <Core/Containers/Vector.hpp>
#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>
#include <Graphics/GraphicsUtils/StateObject.hpp>

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

		void SetRootSignature(D_GRAPHICS_UTILS::RootSignature const& RootSig);
		void SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData);
		void SetBufferSRV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& SRV, UINT64 Offset = 0);
		void SetBufferUAV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& UAV, UINT64 Offset = 0);
		void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle);
		void SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
		void SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
		void SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle);
		void SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[]);
		void SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData);

		INLINE void SetPipelineState(D_GRAPHICS_UTILS::StateObject const& stateObject)
		{
			reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CommandList)->SetPipelineState1(stateObject.GetStateObject());

		}

		INLINE void DispatchRays(D3D12_DISPATCH_RAYS_DESC const* desc)
		{
			FlushResourceBarriers();
			m_DynamicViewDescriptorHeap.CommitComputeRootDescriptorTables(m_CommandList);
			m_DynamicSamplerDescriptorHeap.CommitComputeRootDescriptorTables(m_CommandList);
			reinterpret_cast<ID3D12GraphicsCommandList4*>(m_CommandList)->DispatchRays(desc);
		}

	};

	INLINE void RayTracingCommandContext::SetRootSignature(D_GRAPHICS_UTILS::RootSignature const& RootSig)
	{
		if (RootSig.GetSignature() == m_CurComputeRootSignature)
			return;

		m_CommandList->SetComputeRootSignature(m_CurComputeRootSignature = RootSig.GetSignature());

		m_DynamicViewDescriptorHeap.ParseComputeRootSignature(RootSig);
		m_DynamicSamplerDescriptorHeap.ParseComputeRootSignature(RootSig);
	}

	INLINE void RayTracingCommandContext::SetDynamicConstantBufferView(UINT RootIndex, size_t BufferSize, const void* BufferData)
	{
		D_ASSERT(BufferData != nullptr && D_MEMORY::IsAligned(BufferData, 16));
		D_GRAPHICS_MEMORY::DynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
		//SIMDMemCopy(cb.DataPtr, BufferData, Math::AlignUp(BufferSize, 16) >> 4);
		memcpy(cb.DataPtr, BufferData, BufferSize);
		m_CommandList->SetComputeRootConstantBufferView(RootIndex, cb.GpuAddress);
	}

	INLINE void RayTracingCommandContext::SetBufferSRV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& SRV, UINT64 Offset)
	{
		D_ASSERT((SRV.GetUsageState() & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE) != 0);
		m_CommandList->SetComputeRootShaderResourceView(RootIndex, SRV.GetGpuVirtualAddress() + Offset);
	}

	INLINE void RayTracingCommandContext::SetBufferUAV(UINT RootIndex, const D_GRAPHICS_BUFFERS::GpuBuffer& UAV, UINT64 Offset)
	{
		D_ASSERT((UAV.GetUsageState() & D3D12_RESOURCE_STATE_UNORDERED_ACCESS) != 0);
		m_CommandList->SetComputeRootUnorderedAccessView(RootIndex, UAV.GetGpuVirtualAddress() + Offset);
	}

	INLINE void RayTracingCommandContext::SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE FirstHandle)
	{
		m_CommandList->SetComputeRootDescriptorTable(RootIndex, FirstHandle);
	}

	INLINE void RayTracingCommandContext::SetDynamicDescriptor(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
	{
		SetDynamicDescriptors(RootIndex, Offset, 1, &Handle);
	}

	INLINE void RayTracingCommandContext::SetDynamicDescriptors(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		m_DynamicViewDescriptorHeap.SetComputeDescriptorHandles(RootIndex, Offset, Count, Handles);
	}

	INLINE void RayTracingCommandContext::SetDynamicSampler(UINT RootIndex, UINT Offset, D3D12_CPU_DESCRIPTOR_HANDLE Handle)
	{
		SetDynamicSamplers(RootIndex, Offset, 1, &Handle);
	}

	INLINE void RayTracingCommandContext::SetDynamicSamplers(UINT RootIndex, UINT Offset, UINT Count, const D3D12_CPU_DESCRIPTOR_HANDLE Handles[])
	{
		m_DynamicSamplerDescriptorHeap.SetComputeDescriptorHandles(RootIndex, Offset, Count, Handles);
	}

	INLINE void RayTracingCommandContext::SetDynamicSRV(UINT RootIndex, size_t BufferSize, const void* BufferData)
	{
		D_ASSERT(BufferData != nullptr && D_MEMORY::IsAligned(BufferData, 16));
		D_GRAPHICS_MEMORY::DynAlloc cb = m_CpuLinearAllocator.Allocate(BufferSize);
		D_MEMORY::SIMDMemCopy(cb.DataPtr, BufferData, D_MEMORY::AlignUp(BufferSize, 16) >> 4);
		m_CommandList->SetComputeRootShaderResourceView(RootIndex, cb.GpuAddress);
	}

}