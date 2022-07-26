#pragma once
#include "GraphicsUtils/UploadBuffer.hpp"
#include "Geometry/Mesh.hpp"
#include "FrameResource.hpp"

#include <Utils/Common.hpp>
#include <Math/VectorMath.hpp>

using namespace Darius::Renderer::GraphicsUtils;
using namespace Darius::Math;
using namespace D_RENDERER_GEOMETRY;

using namespace Microsoft::WRL;

#define D_RENDERER_FRAME_RESOUCE Darius::Renderer::ConstantFrameResource

namespace Darius::Renderer::ConstantFrameResource
{
	static constexpr size_t gNumFrameResources = 3;

	ALIGN_DECL_256 struct GlobalConstants
	{
		XMFLOAT4X4			View;
		XMFLOAT4X4			InvView;
		XMFLOAT4X4			Proj;
		XMFLOAT4X4			InvProj;
		XMFLOAT4X4			ViewProj;
		XMFLOAT4X4			InvViewProj;
		XMFLOAT3			CameraPos;
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
		XMFLOAT4X4				mWorld;
	};

	// Lightweight structure stores parameters to draw a shape.
	struct RenderItem
	{
		RenderItem() = default;

		// World matrix of the shape that describes the object's local space
		// relative to the world space, which defines the position,
		// orientation and scale of the object in the world.
		Matrix4						World = Matrix4::Identity();

		// Dirty flag indicating the object data has changed and we need
		// to update thhe constant buffer. Because we have an object
		// cbuffer for each FrameResource, we have to apply the
		// uupdate to each FrameResource. Thus, when we modify object data we
		// should set.
		// NumFramesDirty = mNumFrameResources so that each frame resource
		// gets the update.
		int							NumFramesDirty = gNumFrameResources;

		// Index into GPU constant buffer corresponding to the objectCB
		// for this render item.
		UINT						ObjCBIndex = (UINT)-1;

		// Geometry associated with this render-item. Note that multiple
		// render-items can share the same goemetry.
		Mesh*						Mesh = nullptr;

		// Primitive topology.
		D3D12_PRIMITIVE_TOPOLOGY	PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// DrawIndexedInstance parameters.
		UINT						IndexCount = 0;
		UINT						StartIndexLocation = 0;
		int							BaseVertexLocation = 0;
	};

	// Stores the resources needed for the CPU to build the command lists
	// for a frame
	struct FrameResource : ::NonCopyable
	{
	public:
		FrameResource() = delete;
		FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
		FrameResource(const FrameResource& rhs) = delete;
		~FrameResource();

		void ReinitializeMeshCB(ID3D12Device* device, UINT objectCount);

		// We cannot reset the allocator until the GPU is done processing the
		// commands. So each frame needs their own allocator.
		ComPtr<ID3D12CommandAllocator>					CmdListAlloc;

		// We cannot update a cbuffer until the GPU is done processing the
		// commands that reference it. So each frame needs their own cbuffers.
		std::unique_ptr<D_RENDERER_UTILS::UploadBuffer<GlobalConstants>>	GlobalCB = nullptr;
		std::unique_ptr<D_RENDERER_UTILS::UploadBuffer<MeshConstants>>	MeshCB = nullptr;

		// Fence value to mark commands up to this fence point. This lets us
		// check if these frame resources are still in use by the GPU.
		UINT64											Fence = 0;
	};

}