#pragma once

#define D_GRAPHICS Darius::Graphics

namespace Darius::Graphics
{
	void Initialize();
	void Shutdown();

	D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1);
}