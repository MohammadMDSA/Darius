#include "pch.hpp"
#include "FrameResource.hpp"

namespace Darius::Renderer::ConstantFrameResource
{
	FrameResource::FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount)
	{
		D_HR_CHECK(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(CmdListAlloc.GetAddressOf())));

		GlobalCB = std::make_unique<UploadBuffer<GlobalConstants>>(device, passCount, true);
		ReinitializeMeshCB(device, objectCount);
	}

	FrameResource::~FrameResource() { }

	void FrameResource::ReinitializeMeshCB(ID3D12Device* device, UINT objectCount)
	{
		if (objectCount == mNumObjs)
			return;
		mNumObjs = objectCount;
		MeshCB = std::make_unique<UploadBuffer<MeshConstants>>(device, objectCount, true);
	}
}