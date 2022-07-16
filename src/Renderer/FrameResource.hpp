#pragma once
#include "GraphicsUtils/UploadBuffer.hpp"
#include "DeviceResources.hpp"
#include "Mesh.hpp"

#include <Utils/Common.hpp>
#include <Math/VectorMath.hpp>

using namespace Darius::Renderer::GraphicsUtils;
using namespace Darius::Renderer::DeviceResource;
using namespace Darius::Math;
using namespace Microsoft::WRL;

namespace Darius::Renderer
{
	ALIGN_DECL_256 struct GlobalConstants
	{
		Matrix4				mView = Matrix4::Identity();
		Matrix4				mInvView = Matrix4::Identity();
		Matrix4				mProj = Matrix4::Identity();
		Matrix4				mInvProj = Matrix4::Identity();
		Matrix4				mViewProj = Matrix4::Identity();
		Matrix4				mInvViewProj = Matrix4::Identity();
		Vector3				mCameraPos = Vector3(0);
		float				mcbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2	mRenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2	mInvRenderTargetSize = { 0.0f, 0.0f };
		float				mNearZ = 0.0f;
		float				mFarZ = 0.0f;
		float				mTotalTime = 0.0f;
		float				mDeltaTime = 0.0f;
	};

	ALIGN_DECL_256 struct MeshConstants
	{
		Matrix4				mWorld;
	};

	// Lightweight structure stores parameters to draw a shape.
	struct RenderItem
	{
		RenderItem() = default;

		// World matrix of the shape that describes the object's local space
		// relative to the world space, which defines the position,
		// orientation and scale of the object in the world.
		Matrix4						mWorld = Matrix4::Identity();

		// Dirty flag indicating the object data has changed and we need
		// to update thhe constant buffer. Because we have an object
		// cbuffer for each FrameResource, we have to apply the
		// uupdate to each FrameResource. Thus, when we modify object data we
		// should set.
		// NumFramesDirty = mNumFrameResources so that each frame resource
		// gets the update.
		int							mNumFramesDirty = DeviceResources::gNumFrameResources;

		// Index into GPU constant buffer corresponding to the objectCB
		// for this render item.
		UINT						mObjCBIndex = -1;

		// Geometry associated with this render-item. Note that multiple
		// render-items can share the same goemetry.
		Mesh*						mMesh = nullptr;

		// Primitive topology.
		D3D12_PRIMITIVE_TOPOLOGY	mPrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// DrawIndexedInstance parameters.
		UINT						mIndexCount = 0;
		UINT						mStartIndexLocation = 0;
		int							mBaseVertexLocation = 0;
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