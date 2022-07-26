#include "pch.hpp"
#include "GraphicsCore.hpp"
#include "GraphicsUtils/Memory/DescriptorHeap.hpp"
#include "RenderDeviceManager.hpp"

#include <Utils/Assert.hpp>

using namespace D_GRAPHICS_MEMORY;

namespace Darius::Graphics
{
	bool _initialized = false;
	CommandListManager					CommandManager;
	DescriptorAllocator					DescriptorAllocators[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV
	};
	CommandSignature					DispatchIndirectCommandSignature(1);
	CommandSignature					DrawIndirectCommandSignature(1);

	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;
		CommandManager.Create(D_RENDERER_DEVICE::GetDevice());

		DispatchIndirectCommandSignature[0].Dispatch();
		DispatchIndirectCommandSignature.Finalize();

		DrawIndirectCommandSignature[0].Draw();
		DrawIndirectCommandSignature.Finalize();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
		for (size_t i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++)
		{
			DescriptorAllocators[i].DestroyAll();
		}
		CommandManager.Shutdown();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count)
	{
		return DescriptorAllocators[type].Allocate(count);
	}

	CommandListManager* GetCommandManager()
	{
		D_ASSERT(_initialized);
		return &CommandManager;
	}

	const CommandSignature& GetDispatchIndirectCommandSignature()
	{
		D_ASSERT(_initialized);
		return DispatchIndirectCommandSignature;
	}

	const CommandSignature& GetDrawIndirectCommandSignature()
	{
		D_ASSERT(_initialized);
		return DrawIndirectCommandSignature;
	}
}