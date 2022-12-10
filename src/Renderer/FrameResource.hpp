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
		float IBLRange;
		float IBLBias;
	};

	ALIGN_DECL_256 struct MeshConstants
	{
		Matrix4					mWorld;
		Matrix3					mWorldIT;
	};

	struct Joint
	{
		XMFLOAT4X4				mWorld;
		XMFLOAT3X3				mWorldIT;
	};

	// Color structure for color batches
	ALIGN_DECL_256 struct ColorConstants
	{
		XMFLOAT4		Color;
	};

	ALIGN_DECL_256 struct MaterialConstants
	{
		XMFLOAT4					DifuseAlbedo = { 1.f, 1.f, 1.f, 1.f };
		XMFLOAT3					FresnelR0 = { 0.56f, 0.56f, 0.56f };
		int _pad1;
		XMFLOAT3					Emissive = { 0.f, 0.f, 0.f };
		int _pad2;
		XMFLOAT2					MetallicRoughness = { 0.f, 0.2f };
		int							TextureStatusMask = 0;
	};

	// Lightweight structure stores parameters to draw a shape.
	struct RenderItem
	{
		enum PSOFlags : uint16_t
		{
			HasPosition    = 0x001,
			HasNormal      = 0x002,
			HasTangent     = 0x004,
			HasUV0         = 0x008,
			HasUV1         = 0x010,
			AlphaBlend     = 0x020,
			AlphaTest      = 0x040,
			TwoSided       = 0x080,
			HasSkin        = 0x100,
			Wireframe	   = 0x200,
			ColorOnly	   = 0x400
		};

		RenderItem() = default;

		// Mesh constants GPU Address
		D3D12_GPU_VIRTUAL_ADDRESS		MeshCBV;

		// Material or color
		union
		{
			struct MaterialData
			{
				D3D12_GPU_VIRTUAL_ADDRESS	MaterialCBV;
				D3D12_GPU_DESCRIPTOR_HANDLE	MaterialSRV;
			} Material;

			XMFLOAT4					Color = { 1.f, 1.f, 1.f, 1.f };
		};

		// Geometry associated with this render-item. Note that multiple
		// render-items can share the same goemetry.
		Mesh const* Mesh = nullptr;

		// Primitive topology.
		D3D12_PRIMITIVE_TOPOLOGY	PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// DrawIndexedInstance parameters.
		UINT						IndexCount = 0;
		UINT						StartIndexLocation = 0;
		int							BaseVertexLocation = 0;

		Joint const* mJointData = nullptr;
		int							mNumJoints = 0;

		uint16_t					PsoFlags = 0;
		uint16_t					PsoType;
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