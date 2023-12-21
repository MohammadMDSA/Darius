#pragma once

#include "Renderer/RendererCommon.hpp"
#include "Renderer/Resources/TextureResource.hpp"

#include <Core/Containers/Vector.hpp>
#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/DepthBuffer.hpp>
#include <Math/Transform.hpp>
#include <Math/Camera/Camera.hpp>

#define D_RENDERER_RAST Darius::Renderer::Rasterization

namespace Darius::Renderer::Rasterization
{
	enum RootBindings
	{
		kMeshConstantsVS,		// Holds mesh constants only in Vertex Shader
		kMeshConstantsHS,		// Holds mesh constants only in Hull Shader
		kMeshConstantsDS,		// Holds mesh constants only in Domain Shader
		kDomainConstantsDs,		// Holds material constants only in DS
		kMaterialConstantsPs,	// Holds material constants only in PS
		kTextureDsSRVs,			// Domain shader texture resources
		kMaterialSRVs,
		kMaterialSamplers,
		kCommonCBV,				// Holds global constants in all shaders
		kCommonSRVs,
		kSkinMatrices,

		kNumRootBindings		// Just to know how many root binings there are
	};

	enum TextureType
	{
		kBaseColor,
		kMetallic,
		kRoughness,
		kAmbientOcclusion,
		kEmissive,
		kNormal,
		kWorldDisplacement,

		kNumTextures
	};

	enum  RootSignatureTypes
	{
		DefaultRootSig,

		_numRootSig
	};

#pragma warning(push)
#pragma warning(disable:4201)
	class MeshSorter
	{
	public:
		enum BatchType { kDefault, kShadows };
		enum DrawPass { kZPass, kOpaque, kTransparent, kNumPasses };

		MeshSorter(BatchType type)
		{
			m_BatchType = type;
			m_Camera = nullptr;
			m_Viewport = {};
			m_Scissor = {};
			m_NumRTVs = 0;
			m_DSV = nullptr;
			m_DSVCustom = nullptr;
			m_Norm = nullptr;
			m_SortObjects.clear();
			m_SortKeys.clear();
			std::memset(m_PassCounts, 0, sizeof(m_PassCounts));
			m_CurrentPass = kZPass;
			m_CurrentDraw = 0;
		}

		// Copies only render config
		MeshSorter(MeshSorter const& other)
		{
			m_BatchType = other.m_BatchType;
			m_Camera = other.m_Camera;
			m_Viewport = other.m_Viewport;
			m_Scissor = other.m_Scissor;
			m_NumRTVs = other.m_NumRTVs;
			m_DSV = other.m_DSV;
			m_DSVCustom = other.m_DSVCustom;
			m_Norm = other.m_Norm;
			memcpy(m_RTV, other.m_RTV, sizeof(D_GRAPHICS_BUFFERS::ColorBuffer*) * m_NumRTVs);


			m_SortObjects.clear();
			m_SortKeys.clear();

			std::memset(m_PassCounts, 0, sizeof(m_PassCounts));

			m_CurrentPass = kZPass;
			m_CurrentDraw = 0;

		}

		void SetCamera(const D_MATH_CAMERA::BaseCamera& camera) { m_Camera = &camera; }
		void SetViewport(const D3D12_VIEWPORT& viewport) { m_Viewport = viewport; }
		void SetScissor(const D3D12_RECT& scissor) { m_Scissor = scissor; }
		void AddRenderTarget(D_GRAPHICS_BUFFERS::ColorBuffer& RTV)
		{
			D_ASSERT(m_NumRTVs < 8);
			m_RTV[m_NumRTVs++] = &RTV;
		}
		void SetDepthStencilTarget(D_GRAPHICS_BUFFERS::DepthBuffer& DSV, D_GRAPHICS_BUFFERS::DepthBuffer* DSVCustom)
		{
			m_DSV = &DSV;
			m_DSVCustom = DSVCustom;

			if (m_DSVCustom)
			{
				D_ASSERT(m_DSV->GetWidth() == m_DSVCustom->GetWidth());
				D_ASSERT(m_DSV->GetHeight() == m_DSVCustom->GetHeight());
			}
		}

		INLINE void SetNormalTarget(D_GRAPHICS_BUFFERS::ColorBuffer& normal) { m_Norm = &normal; }

		const D_MATH_CAMERA::Frustum& GetWorldFrustum() const { return m_Camera->GetWorldSpaceFrustum(); }
		const D_MATH_CAMERA::Frustum& GetViewFrustum() const { return m_Camera->GetViewSpaceFrustum(); }
		const D_MATH::Matrix4& GetViewMatrix() const { return m_Camera->GetViewMatrix(); }

		void AddMesh(D_RENDERER::RenderItem const& renderItem, float distance);

		void Sort();

		void SetupDefaultBatchTypeRenderTargetsAfterCustomDepth(D_GRAPHICS::GraphicsContext& context);

		void RenderMeshes(DrawPass pass,
			D_GRAPHICS::GraphicsContext& context,
			D_GRAPHICS_BUFFERS::ColorBuffer* ssao,
			D_RENDERER::GlobalConstants& globals);

		size_t CountObjects() const { return m_SortObjects.size(); }

		void Reset()
		{
			m_CurrentPass = kZPass;
			m_CurrentDraw = 0;
		}

	private:

		struct SortKey
		{
			union
			{
				uint64_t value;
				struct
				{
					uint64_t objectIdx : 16;
					uint64_t psoIdx : 12;
					uint64_t key : 32;
					uint64_t passID : 4;
				};
			};
		};

		struct SortObject
		{
			D_RENDERER::RenderItem const renderItem;
		};

		D_CONTAINERS::DVector<SortObject> m_SortObjects;
		D_CONTAINERS::DVector<uint64_t> m_SortKeys;
		BatchType m_BatchType;
		uint32_t m_PassCounts[kNumPasses];
		DrawPass m_CurrentPass;
		uint32_t m_CurrentDraw;

		const D_MATH_CAMERA::BaseCamera* m_Camera;
		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_Scissor;
		uint32_t m_NumRTVs;
		D_GRAPHICS_BUFFERS::ColorBuffer* m_RTV[8];
		D_GRAPHICS_BUFFERS::ColorBuffer* m_Norm;
		D_GRAPHICS_BUFFERS::DepthBuffer* m_DSV;
		D_GRAPHICS_BUFFERS::DepthBuffer* m_DSVCustom;
	};

	struct PsoConfig
	{
		UINT32	PsoFlags : 16 = 0u;
		UINT32	HSIndex : 8 = 0u;
		UINT32	DSIndex : 8 = 0u;
		UINT32	VSIndex : 10 = 0u;
		UINT32	PSIndex : 12 = 0u;
		UINT32	GSIndex : 10 = 0u;
		D3D12_INPUT_LAYOUT_DESC InputLayout =
		{
			nullptr,
			0
		};
	};

	D_STATIC_ASSERT(sizeof(PsoConfig) == 8u + sizeof(D3D12_INPUT_LAYOUT_DESC));

	void Initialize(D_SERIALIZATION::Json const& settings);
	void Shutdown();
	void Update(D_GRAPHICS::CommandContext& context);
	void Render(std::wstring const& jobId, SceneRenderContext& rContext, std::function<void()> postAntiAliasing = nullptr);

#ifdef _D_EDITOR
	bool OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);

	UINT16 const& GetForcedPsoFlags();
	void					SetForceWireframe(bool val);
#endif

	D_GRAPHICS_UTILS::RootSignature& GetRootSignature(RootSignatureTypes type);

	// Set IBL properties
	void					SetIBLTextures(D_RENDERER::TextureResource* diffuseIBL, D_RENDERER::TextureResource* specularIBL);
	void					SetIBLBias(float LODBias);

	// PSO Getter
	UINT					GetPso(PsoConfig const&);

	// Allocating from heaps
	D_GRAPHICS_MEMORY::DescriptorHandle AllocateTextureDescriptor(UINT count = 1);
	D_GRAPHICS_MEMORY::DescriptorHandle AllocateSamplerDescriptor(UINT count = 1);

	D_CONTAINERS::DVector<D_GRAPHICS_UTILS::GraphicsPSO> const& GetPsos();
}
#pragma warning(pop)