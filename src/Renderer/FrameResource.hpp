#pragma once
#include "Geometry/Mesh.hpp"
#include "FrameResource.hpp"
#include "LightManager.hpp"

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
		XMFLOAT4			AmbientLight;

		D_LIGHT::LightData	Lights[256];
	};

	ALIGN_DECL_256 struct MeshConstants
	{
		XMFLOAT4X4				mWorld;
	};

	// Color structure for color batches
	ALIGN_DECL_256 struct ColorConstants
	{
		XMFLOAT4		Color;
	};

	ALIGN_DECL_256 struct MaterialConstants
	{
		XMFLOAT4					DifuseAlbedo = { 1.f, 1.f, 1.f, 1.f };
		XMFLOAT3					FresnelR0 = { 0.f, 0.f, 0.f };
		float						Roughness = 0.2;
	};

	// Lightweight structure stores parameters to draw a shape.
	struct RenderItem
	{
		RenderItem() = default;

		// Dirty flag indicating the object data has changed and we need
		// to update thhe constant buffer. Because we have an object
		// cbuffer for each FrameResource, we have to apply the
		// uupdate to each FrameResource. Thus, when we modify object data we
		// should set.
		// NumFramesDirty = mNumFrameResources so that each frame resource
		// gets the update.
		int								NumFramesDirty = gNumFrameResources;

		// Mesh constants GPU Address
		D3D12_GPU_VIRTUAL_ADDRESS		MeshCBV;

		// Material or color
		union
		{
			D3D12_GPU_VIRTUAL_ADDRESS	MaterialCBV;
			XMFLOAT4					Color = { 1.f, 1.f, 1.f, 1.f };
		};

		// Geometry associated with this render-item. Note that multiple
		// render-items can share the same goemetry.
		const Mesh* Mesh = nullptr;

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
		~FrameResource() = default;

		// We cannot reset the allocator until the GPU is done processing the
		// commands. So each frame needs their own allocator.
		ComPtr<ID3D12CommandAllocator>					CmdListAlloc;

		// Fence value to mark commands up to this fence point. This lets us
		// check if these frame resources are still in use by the GPU.
		UINT64											Fence = 0;
	private:
		UINT	mNumObjs = 0;
	};

}