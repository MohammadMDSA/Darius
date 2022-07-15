#pragma once
#include "GraphicsUtils/UploadBuffer.hpp"

#include <Utils/Common.hpp>
#include <Math/VectorMath.hpp>

using namespace Darius::Renderer::GraphicsUtils;
using namespace Darius::Math;
using namespace Microsoft::WRL;

namespace Darius::Renderer
{
	ALIGN_DECL_256 struct GlobalConstants
	{
		Matrix4				View = Matrix4::Identity();
		Matrix4				InvView = Matrix4::Identity();
		Matrix4				Proj = Matrix4::Identity();
		Matrix4				InvProj = Matrix4::Identity();
		Matrix4				ViewProj = Matrix4::Identity();
		Matrix4				InvViewProj = Matrix4::Identity();
		Vector3				CameraPos = Vector3(0);
		float				cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2	RenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2	InvRenderTargetSize = { 0.0f, 0.0f };
		float				NearZ = 0.0f;
		float				FarZ = 0.0f;
		float				TotalTime = 0.0f;
		float				DeltaTime = 0.0f;
	};

	ALIGN_DECL_256 struct MeshConstants
	{
		Matrix4				World;
	};

	// Stores the resources needed for the CPU to build the command lists
	// for a frame
	struct FrameResource : NonCopyable
	{
	public:
		FrameResource() = delete;
		FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
		~FrameResource();

		// We cannot reset the allocator until the GPU is done processing the
		// commands. So each frame needs their own allocator.
		ComPtr<ID3D12CommandAllocator>					mCmdListAlloc;

		// Buffers to draw scene on
		ComPtr<ID3D12Resource>							mRenderTarget;

		// We cannot update a cbuffer until the GPU is done processing the
		// commands that reference it. So each frame needs their own cbuffers.
		std::unique_ptr<UploadBuffer<GlobalConstants>>	mGlobalCB = nullptr;
		std::unique_ptr<UploadBuffer<MeshConstants>>	mMeshCB = nullptr;

		// Fence value to mark commands up to this fence point. This lets us
		// check if these frame resources are still in use by the GPU.
		UINT64											mFence = 0;
	};

}