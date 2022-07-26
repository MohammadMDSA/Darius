#pragma once

#include "GraphicsUtils/CommandListManager.hpp"
#include "CommandSignature.hpp"

#define D_GRAPHICS Darius::Graphics

using namespace D_GRAPHICS_UTILS;

namespace Darius::Graphics
{
	void Initialize();
	void Shutdown();

	D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1);

	CommandListManager* GetCommandManager();
	const CommandSignature& GetDispatchIndirectCommandSignature();
	const CommandSignature& GetDrawIndirectCommandSignature();
}