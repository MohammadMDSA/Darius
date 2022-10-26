#pragma once

#include "RenderDeviceManager.hpp"
#include "CommandContext.hpp"
#include "CommandContext.hpp"
#include "GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "GraphicsUtils/Buffers/DepthBuffer.hpp"

#include <Core/Containers/Vector.hpp>
#include <Math/Transform.hpp>
#include <Math/Camera/Camera.hpp>

#define D_RENDERER Darius::Renderer
#define D_DEVICE D_RENDERER_DEVICE

using namespace D_MATH;
using namespace D_MATH::Camera;
using namespace D_RENDERER_DEVICE;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_GRAPHICS;

namespace Darius::Renderer
{
	extern UINT PassCbvOffset;

	enum RootBindings
	{
		kMeshConstants,			// Holds mesh constants only in VS
		kMaterialConstants,		// Holds material constants only in PS
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
		kRoughness,
		kOcculusion,
		kEmissive,
		kNormal,

		kNumTextures
	};

	enum PipelineStateTypes
	{
		OpaquePso,
		TransparentPso,
		WireframePso,
		SkinnedOpaquePso,
		SkinnedTransparentPso,
		SkinnedWireframePso,
		ColorPso,
		ColorWireframePso,
		ColorWireframeTwoSidedPso,

		_numPso
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
			m_SortObjects.clear();
			m_SortKeys.clear();
			std::memset(m_PassCounts, 0, sizeof(m_PassCounts));
			m_CurrentPass = kZPass;
			m_CurrentDraw = 0;
		}

		void SetCamera(const BaseCamera& camera) { m_Camera = &camera; }
		void SetViewport(const D3D12_VIEWPORT& viewport) { m_Viewport = viewport; }
		void SetScissor(const D3D12_RECT& scissor) { m_Scissor = scissor; }
		void AddRenderTarget(D_GRAPHICS_BUFFERS::ColorBuffer& RTV)
		{
			D_ASSERT(m_NumRTVs < 8);
			m_RTV[m_NumRTVs++] = &RTV;
		}
		void SetDepthStencilTarget(D_GRAPHICS_BUFFERS::DepthBuffer& DSV) { m_DSV = &DSV; }

		const Frustum& GetWorldFrustum() const { return m_Camera->GetWorldSpaceFrustum(); }
		const Frustum& GetViewFrustum() const { return m_Camera->GetViewSpaceFrustum(); }
		const Matrix4& GetViewMatrix() const { return m_Camera->GetViewMatrix(); }

		void AddMesh(RenderItem const& renderItem, float distance);

		void Sort();

		void RenderMeshes(DrawPass pass, D_GRAPHICS::GraphicsContext& context, GlobalConstants& globals);

		size_t CountObjects() const { return m_SortObjects.size(); }

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
			RenderItem const renderItem;
		};

		D_CONTAINERS::DVector<SortObject> m_SortObjects;
		D_CONTAINERS::DVector<uint64_t> m_SortKeys;
		BatchType m_BatchType;
		uint32_t m_PassCounts[kNumPasses];
		DrawPass m_CurrentPass;
		uint32_t m_CurrentDraw;

		const BaseCamera* m_Camera;
		D3D12_VIEWPORT m_Viewport;
		D3D12_RECT m_Scissor;
		uint32_t m_NumRTVs;
		D_GRAPHICS_BUFFERS::ColorBuffer* m_RTV[8];
		D_GRAPHICS_BUFFERS::DepthBuffer* m_DSV;
	};

	void Initialize();
	void Shutdown();

#ifdef _D_EDITOR
	DescriptorHandle		GetUiTextureHandle(UINT index);
	void					RenderGui(D_GRAPHICS::GraphicsContext& context);
#endif
	// For rendering meshs
	void					RenderMeshes(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems, D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals);

	// For rendering batches like line and debug stuff
	void					RenderBatchs(D_GRAPHICS::GraphicsContext& context, D_CONTAINERS::DVector<RenderItem> const& renderItems, D_RENDERER_FRAME_RESOUCE::GlobalConstants const& globals);

	void					Present(D_GRAPHICS::GraphicsContext& context);

	D_GRAPHICS_UTILS::GraphicsPSO& GetPSO(PipelineStateTypes type);
	D_GRAPHICS_UTILS::RootSignature& GetRootSignature(RootSignatureTypes type);

	DescriptorHandle		AllocateTextureDescriptor(UINT count = 1);
}
#pragma warning(pop)