#pragma once
#include "Geometry/Mesh.hpp"

#include <Utils/Common.hpp>
#include <Math/VectorMath.hpp>

#ifndef D_RENDERER_FRAME_RESOURCE
#define D_RENDERER_FRAME_RESOURCE Darius::Renderer::ConstantFrameResource
#endif // !D_RENDERER_FRAME_RESOURCE

namespace Darius::Renderer::ConstantFrameResource
{
	static constexpr size_t gNumFrameResources = 3;

	ALIGN_DECL_256 struct GlobalConstants
	{
		DirectX::XMFLOAT4X4	View;
		DirectX::XMFLOAT4X4	InvView;
		DirectX::XMFLOAT4X4	Proj;
		DirectX::XMFLOAT4X4	InvProj;
		DirectX::XMFLOAT4X4	ViewProj;
		DirectX::XMFLOAT4X4	InvViewProj;
		DirectX::XMFLOAT4	ShadowTexelSize = { 0.f, 0.f, 0.f, 0.f };
		DirectX::XMFLOAT3	CameraPos;
		float				cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2	RenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2	InvRenderTargetSize = { 0.0f, 0.0f };
		float				NearZ = 0.0f;
		float				FarZ = 0.0f;
		float				TotalTime = 0.0f;
		float				DeltaTime = 0.0f;
		DirectX::XMFLOAT4	AmbientLight;
		float IBLRange;
		float IBLBias;
	};

	ALIGN_DECL_256 struct MeshConstants
	{
		D_MATH::Matrix4			mWorld;
		D_MATH::Matrix3			mWorldIT;
	};

	struct Joint
	{
		DirectX::XMFLOAT4X4				mWorld;
		DirectX::XMFLOAT3X3				mWorldIT;
	};

	// Color structure for color batches
	ALIGN_DECL_256 struct ColorConstants
	{
		DirectX::XMFLOAT4		Color;
	};

#pragma warning(push)
#pragma warning(disable: 4201)
	ALIGN_DECL_256 struct MaterialConstants
	{
		DirectX::XMFLOAT4			DifuseAlbedo = { 1.f, 1.f, 1.f, 1.f };
		DirectX::XMFLOAT3			FresnelR0 = { 0.56f, 0.56f, 0.56f };
		int _pad1;
		DirectX::XMFLOAT3			Emissive = { 0.f, 0.f, 0.f };
		float						Metallic = 0.f;
		float						Roughness = 0.f;
		struct
		{
			UINT						TextureStatusMask : 16 = 0;
			UINT						AlphaCutout : 16 = 0;
		};
	};
#pragma warning(pop)

	// Lightweight structure stores parameters to draw a shape.
	struct RenderItem
	{
		enum PSOFlags : uint16_t
		{
			HasPosition = 0x001,
			HasNormal = 0x002,
			HasTangent = 0x004,
			HasUV0 = 0x008,
			HasUV1 = 0x010,
			AlphaBlend = 0x020,
			AlphaTest = 0x040,
			TwoSided = 0x080,
			HasSkin = 0x100,
			Wireframe = 0x200,
			ColorOnly = 0x400,
			LineOnly = 0x800
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
				D3D12_GPU_DESCRIPTOR_HANDLE SamplersSRV;
			} Material;

			DirectX::XMFLOAT4			Color = { 1.f, 1.f, 1.f, 1.f };
		};

		// Geometry associated with this render-item. Note that multiple
		// render-items can share the same goemetry.
		D_RENDERER_GEOMETRY::Mesh const* Mesh = nullptr;

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

}