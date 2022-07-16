#include "pch.hpp"
#include "FrameResource.hpp"

namespace Darius::Renderer::ConstantFrameResource
{
	FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
	{
		D_HR_CHECK(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(mCmdListAlloc.GetAddressOf())));
	}

	FrameResource::~FrameResource() { }
}