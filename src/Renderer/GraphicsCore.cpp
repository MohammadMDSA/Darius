#include "pch.hpp"
#include "GraphicsCore.hpp"
#include "GraphicsUtils/DescriptorHeap.hpp"

#include <Utils/Assert.hpp>

using namespace D_GRAPHICS_UTILS;

namespace Darius::Graphics
{
	bool _initialized = false;

	DescriptorAllocator					DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV
	};

	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
		for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
		{
			DescriptorAllocators[i].DestroyAll();
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count)
	{
		D_ASSERT(_initialized);
		return DescriptorAllocators[type].Allocate(count);
	}
}