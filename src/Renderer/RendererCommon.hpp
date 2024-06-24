#pragma once

#include "Renderer/Geometry/Mesh.hpp"
#include "Renderer/Resources/TextureResource.hpp"

#include <Core/Containers/List.hpp>
#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/DepthBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/Texture.hpp>
#include <Graphics/PostProcessing/PostProcessing.hpp>
#include <Math/Camera/Camera.hpp>

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	ALIGN_DECL_256 struct GlobalConstants
	{
		DirectX::XMFLOAT4X4	View;
		DirectX::XMFLOAT4X4	InvView;
		DirectX::XMFLOAT4X4	Proj;
		DirectX::XMFLOAT4X4	InvProj;
		DirectX::XMFLOAT4X4	ViewProj;
		DirectX::XMFLOAT4X4	InvViewProj;
		DirectX::XMFLOAT4X4 InvViewProjEyeCenter;
		DirectX::XMFLOAT4	FrustumPlanes[6] = { { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f, 0.f } };
		DirectX::XMFLOAT3	CameraPos = { 0.f, 0.f, 0.f };
		float				cbPerObjectPad1 = 0.0f;
		DirectX::XMFLOAT2	RenderTargetSize = { 0.0f, 0.0f };
		DirectX::XMFLOAT2	InvRenderTargetSize = { 0.0f, 0.0f };
		float				NearZ = 0.0f;
		float				FarZ = 0.0f;
		float				TotalTime = 0.0f;
		float				DeltaTime = 0.0f;
		DirectX::XMFLOAT4	AmbientLight = { 0.f, 0.f, 0.f, 0.f };
		float				IBLRange = 0.f;
		float				IBLBias = 0.f;
	};

	struct RenderItemContext
	{
#if _D_EDITOR
		uint8_t				IsEditor : 1;
		uint8_t				PickerDraw : 1 = false;
		void*				SelectedGameObject;
		uint8_t				StencilOverride = false;
#endif
		bool				Shadow = false;
	};

	ALIGN_DECL_256 struct MeshConstants
	{
		D_MATH::Matrix4			World;
		D_MATH::Matrix3			WorldIT;
		float					Lod = 1.f;
	};

#if _D_EDITOR
	ALIGN_DECL_16 struct PickerPsMeshConstants
	{
		DirectX::XMUINT2		Id;
	};
#endif // _D_EDITOR


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
		float						Roughness = 1.f;
		float						DisplacementAmount = 0.f;
		float						Opacity = 1.f;
		float						Specular = 1.f;
		struct
		{
			UINT					TextureStatusMask : 16 = 0;
			UINT					AlphaCutout : 16 = 0;
		};
	};
#pragma warning(pop)

	// Lightweight structure stores parameters to draw a shape.
	struct RenderItem
	{
		enum PSOFlags : uint16_t
		{
			HasPosition = 1 << 0,
			HasNormal = 1 << 1,
			HasTangent = 1 << 2,
			HasUV0 = 1 << 3,
			HasUV1 = 1 << 4,
			AlphaBlend = 1 << 5,
			AlphaTest = 1 << 6,
			TwoSided = 1 << 7,
			HasSkin = 1 << 8,
			Wireframe = 1 << 9,
			ColorOnly = 1 << 10,
			LineOnly = 1 << 11,
			PointOnly = 1 << 12,
			DepthOnly = 1 << 13,
			SkipVertexIndex = 1 << 14,
			HasDisplacement = 1 << 15,
		};

		RenderItem() = default;

		// Mesh constants GPU Address
		D3D12_GPU_VIRTUAL_ADDRESS		MeshVsCBV = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		D3D12_GPU_VIRTUAL_ADDRESS		MeshHsCBV = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		D3D12_GPU_VIRTUAL_ADDRESS		MeshDsCBV = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		D3D12_GPU_VIRTUAL_ADDRESS		ParamsDsCBV = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

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

		// Domain Shader Textures
		D3D12_GPU_DESCRIPTOR_HANDLE	TextureDomainSRV = { 0 };

		// Geometry associated with this render-item. Note that multiple
		// render-items can share the same goemetry.
		D_RENDERER_GEOMETRY::Mesh const* Mesh = nullptr;

		// Primitive topology.
		D3D12_PRIMITIVE_TOPOLOGY	PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		// DrawIndexedInstance parameters.
		UINT						IndexCount = 0;
		UINT						StartIndexLocation = 0;
		int							BaseVertexLocation = 0;

		Joint const*				mJointData = nullptr;
		int							mNumJoints = 0;

		UINT						PsoFlags : 16 = 0;
		UINT						StencilValue : 8 = 0;
		UINT						StencilEnable : 1 = 0;
		UINT						CustomDepth : 1 = 0;
		UINT						PsoType = 0;
		UINT						DepthPsoIndex = 0;

	};

	struct RenderViewBuffers
	{
		// Main Buffers
		D_GRAPHICS_BUFFERS::ColorBuffer				SceneTexture;
		D_GRAPHICS_BUFFERS::DepthBuffer				SceneDepth;
		D_GRAPHICS_BUFFERS::DepthBuffer				CustomDepth;
		D_GRAPHICS_BUFFERS::ColorBuffer				SceneNormals;

		// TAA Buffers
		D_GRAPHICS_BUFFERS::ColorBuffer				TemporalColor[2];
		D_GRAPHICS_BUFFERS::ColorBuffer				VelocityBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer				LinearDepth[2];

		// Post Processing Buffers
		// HDR ToneMapping
		D_GRAPHICS_BUFFERS::StructuredBuffer		ExposureBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer				LumaBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer				LumaLR;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer		HistogramBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer				PostEffectsBuffer;
		// Bloom
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV1[2];
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV2[2];
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV3[2];
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV4[2];
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV5[2];

		// Ambient Occlusion Buffers
		D_GRAPHICS_BUFFERS::ColorBuffer				SSAOFullScreen;
		D_GRAPHICS_BUFFERS::ColorBuffer				DepthDownsize1;
		D_GRAPHICS_BUFFERS::ColorBuffer				DepthDownsize2;
		D_GRAPHICS_BUFFERS::ColorBuffer				DepthDownsize3;
		D_GRAPHICS_BUFFERS::ColorBuffer				DepthDownsize4;
		D_GRAPHICS_BUFFERS::ColorBuffer				DepthTiled1;
		D_GRAPHICS_BUFFERS::ColorBuffer				DepthTiled2;
		D_GRAPHICS_BUFFERS::ColorBuffer				DepthTiled3;
		D_GRAPHICS_BUFFERS::ColorBuffer				DepthTiled4;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOMerged1;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOMerged2;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOMerged3;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOMerged4;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOSmooth1;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOSmooth2;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOSmooth3;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOHighQuality1;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOHighQuality2;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOHighQuality3;
		D_GRAPHICS_BUFFERS::ColorBuffer				AOHighQuality4;

		D_GRAPHICS_BUFFERS::ColorBuffer				WorldPos;
		D_GRAPHICS_BUFFERS::ColorBuffer				NormalDepth;

		D_GRAPHICS_BUFFERS::ByteAddressBuffer		FXAAWorkQueue;
		D_GRAPHICS_BUFFERS::TypedBuffer				FXAAColorQueue;

		RenderViewBuffers() :
			FXAAColorQueue(DXGI_FORMAT_R11G11B10_FLOAT)
		{ }

		void Create(uint32_t width, uint32_t height, std::wstring const& contextName, bool customDepthAvailable);
		void Destroy();

		D_GRAPHICS_PP::PostProcessContextBuffers GetPostProcessingBuffers(std::wstring const& contextName);
	};

	struct SceneRenderContext
	{
		D_GRAPHICS_BUFFERS::DepthBuffer&	DepthBuffer;
		D_GRAPHICS_BUFFERS::DepthBuffer*	CustomDepthBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer&	ColorBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer&	NormalBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer&	VelocityBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer*	TemporalColor;
		D_GRAPHICS_BUFFERS::ColorBuffer*	LinearDepth;
		D_GRAPHICS_BUFFERS::ColorBuffer&	SSAOFullScreen;
		D_GRAPHICS_BUFFERS::ColorBuffer&	DepthDownsize1;
		D_GRAPHICS_BUFFERS::ColorBuffer&	DepthDownsize2;
		D_GRAPHICS_BUFFERS::ColorBuffer&	DepthDownsize3;
		D_GRAPHICS_BUFFERS::ColorBuffer&	DepthDownsize4;
		D_GRAPHICS_BUFFERS::ColorBuffer&	DepthTiled1;
		D_GRAPHICS_BUFFERS::ColorBuffer&	DepthTiled2;
		D_GRAPHICS_BUFFERS::ColorBuffer&	DepthTiled3;
		D_GRAPHICS_BUFFERS::ColorBuffer&	DepthTiled4;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOMerged1;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOMerged2;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOMerged3;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOMerged4;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOSmooth1;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOSmooth2;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOSmooth3;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOHighQuality1;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOHighQuality2;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOHighQuality3;
		D_GRAPHICS_BUFFERS::ColorBuffer&	AOHighQuality4;
		D_GRAPHICS_BUFFERS::ColorBuffer&	WorldPos;
		D_GRAPHICS_BUFFERS::ColorBuffer&	NormalDepth;
#if _D_EDITOR
		D_GRAPHICS_BUFFERS::DepthBuffer*	PickerDepthBuffer = nullptr;
		D_GRAPHICS_BUFFERS::ColorBuffer*	PickerColorBuffer = nullptr;
#endif
		D_GRAPHICS::CommandContext&			CommandContext;
		D_MATH_CAMERA::Camera const&		Camera;
		GlobalConstants&					Globals;
		D_CONTAINERS::DVector<D_CONTAINERS::DVector<RenderItem> const*> const AdditionalRenderItems = {};
		TextureResource*					RadianceIBL;
		TextureResource*					IrradianceIBL;
		RenderItemContext const&			RenderItemCtx;
		uint32_t							DrawSkybox : 1;
#if _D_EDITOR
		uint32_t							RenderPickerData : 1 = false;
#endif

		INLINE static SceneRenderContext MakeFromBuffers(
			RenderViewBuffers& renderBuffers,
			bool customDepth,
			D_GRAPHICS::CommandContext& commandContext,
			D_MATH_CAMERA::Camera const& camera,
			GlobalConstants& globals,
			RenderItemContext const& renderItemContext,
			D_CONTAINERS::DVector<D_CONTAINERS::DVector<RenderItem> const*> const& additionalRenderItems = {}
			)
		{
			return SceneRenderContext
			{
				.DepthBuffer = renderBuffers.SceneDepth,
				.CustomDepthBuffer = customDepth ? &renderBuffers.CustomDepth : nullptr,
				.ColorBuffer = renderBuffers.SceneTexture,
				.NormalBuffer = renderBuffers.SceneNormals,
				.VelocityBuffer = renderBuffers.VelocityBuffer,
				.TemporalColor = renderBuffers.TemporalColor,
				.LinearDepth = renderBuffers.LinearDepth,
				.SSAOFullScreen = renderBuffers.SSAOFullScreen,
				.DepthDownsize1 = renderBuffers.DepthDownsize1,
				.DepthDownsize2 = renderBuffers.DepthDownsize2,
				.DepthDownsize3 = renderBuffers.DepthDownsize3,
				.DepthDownsize4 = renderBuffers.DepthDownsize4,
				.DepthTiled1 = renderBuffers.DepthTiled1,
				.DepthTiled2 = renderBuffers.DepthTiled2,
				.DepthTiled3 = renderBuffers.DepthTiled3,
				.DepthTiled4 = renderBuffers.DepthTiled4,
				.AOMerged1 = renderBuffers.AOMerged1,
				.AOMerged2 = renderBuffers.AOMerged2,
				.AOMerged3 = renderBuffers.AOMerged3,
				.AOMerged4 = renderBuffers.AOMerged4,
				.AOSmooth1 = renderBuffers.AOSmooth1,
				.AOSmooth2 = renderBuffers.AOSmooth2,
				.AOSmooth3 = renderBuffers.AOSmooth3,
				.AOHighQuality1 = renderBuffers.AOHighQuality1,
				.AOHighQuality2 = renderBuffers.AOHighQuality2,
				.AOHighQuality3 = renderBuffers.AOHighQuality3,
				.AOHighQuality4 = renderBuffers.AOHighQuality4,
				.WorldPos = renderBuffers.WorldPos,
				.NormalDepth = renderBuffers.NormalDepth,
				.CommandContext = commandContext,
				.Camera = camera,
				.Globals = globals,
				.AdditionalRenderItems = additionalRenderItems,
				.RenderItemCtx = renderItemContext
			};
		}
	};
}