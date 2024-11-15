#pragma once

#include "RenderPassManager.hpp"

#include <Core/Containers/Vector.hpp>
#include <Core/Containers/Map.hpp>
#include <Core/Memory/Allocators/MemoryPool.hpp>
#include <Core/RefCounting/Ref.hpp>
#include <Core/StringId.hpp>

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Core::Filesystem
{
	class Path;
}

namespace Darius::Core::Memory
{
	class Allocator;
	class StackAllocator;
	class LinearAllocator;
	class MallocAllocator;
}

namespace Darius::Graphics
{
	class CommandContext;

	namespace Utils::Buffers
	{
		class ColorBuffer;
		class DepthBuffer;
	}

}

namespace Darius::Renderer
{
	class RenderPass;
	class RenderPassManager;

	typedef uint32_t					FrameGraphHandle;

	struct FrameGraphResourceHandle
	{
		FrameGraphHandle				Index;
	};

	struct FrameGraphNodeHandle
	{
		FrameGraphHandle				Index;
	};

	enum class FrameGraphResourceType
	{
		Invalid = -1,

		Buffer = 0,
		Texture,
		Attachment,

		// Creates graph edge but is not actually modified by the pass
		Reference,
		ShadingRate
	};

	enum class FrameGraphTextureType
	{
		Texture1D,
		Texture2D,
		Texture3D,
		Texture1DArray,
		Texture2DArray,
		TextureCubeArray,
		Count
	};

	enum class FramGraphTextureOperation
	{
		DontCare,
		Load,
		Clear,
		Count
	};

	struct FrameGraphResourceInfo
	{
		bool						External = false;

		union
		{
			struct
			{
				size_t					Size;
				D3D12_RESOURCE_FLAGS	Flags;

				uint32_t				Handle;
			} Buffer;

			struct
			{
				uint32_t					Width = 1;
				uint32_t					Height = 1;
				uint32_t					Depth = 1;
				uint32_t					ArraySize = 1;

				DXGI_FORMAT					Format = DXGI_FORMAT_UNKNOWN;
				FrameGraphTextureType		Type = FrameGraphTextureType::Texture2D;
				std::string					DebugName;

				FramGraphTextureOperation	LoadOp;
				float						ClearColor[4];

				uint32_t					Handle;
			} Texture;
		};

		FrameGraphResourceInfo& operator=(FrameGraphResourceInfo const& other)
		{
			External = other.External;
			Texture = other.Texture;
		}
	};

	struct FrameGraphResource
	{
		FrameGraphResourceType		Type;
		FrameGraphResourceInfo		ResouceInfo;

		FrameGraphNodeHandle		Producer;
		FrameGraphResourceHandle	OutputHandle;

		uint32_t					RefCount = 0;

		D_CORE::StringId			Name = ""_SId;
	};

	struct FrameGraphResourceInputCreation
	{
		FrameGraphResourceType		Type;
		FrameGraphResourceInfo		ResourceInfo;

		D_CORE::StringId			Name = ""_SId;
	};

	struct FrameGraphResourceOutputCreation
	{
		FrameGraphResourceType		Type;
		FrameGraphResourceInfo		ResourceInfo;

		D_CORE::StringId			Name = ""_SId;
	};

	struct FrameGraphNodeCreation
	{
		D_CONTAINERS::DVector<FrameGraphResourceInputCreation>		Input;
		D_CONTAINERS::DVector<FrameGraphResourceOutputCreation>		Output;

		bool														Enabled;

		D_CORE::StringId											Name = ""_SId;
	};

	struct FrameGraphFrameBuffer
	{
		RenderPassHandle			RenderPass;
		uint16_t					NumRenderTargets = 0u;
		D_CORE::Ref<Darius::Graphics::Utils::Buffers::ColorBuffer> OutputTextures[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		D_CORE::Ref<Darius::Graphics::Utils::Buffers::DepthBuffer> DepthStencilTexture = nullptr;

		uint16_t					Width;
		uint16_t					Height;

		float						ScaleX = 1.f;
		float						ScaleY = 1.f;
		uint8_t						Resize = 1u;

		D_CORE::StringId			Name = ""_SId;

		FrameGraphFrameBuffer& Reset()
		{
			NumRenderTargets = 0;
			Name = ""_SId;
			DepthStencilTexture = nullptr;

			Resize = 0;
			ScaleX = 1.f;
			ScaleY = 1.f;

			return *this;
		}

		FrameGraphFrameBuffer& AddRenderTexture(D_CORE::Ref<Darius::Graphics::Utils::Buffers::ColorBuffer> texture)
		{
			OutputTextures[NumRenderTargets++];
			return *this;
		}

		FrameGraphFrameBuffer& SetDepthStencilTexture(D_CORE::Ref< Darius::Graphics::Utils::Buffers::DepthBuffer> depthStencil)
		{
			DepthStencilTexture = depthStencil;
			return *this;
		}

		FrameGraphFrameBuffer& SetScaling(float x, float y, uint8_t resize)
		{
			ScaleX = x;
			ScaleY = y;
			Resize = resize;

			return *this;
		}

		FrameGraphFrameBuffer& SetName(D_CORE::StringId const& name)
		{
			Name = name;
			return *this;
		}
	};

	struct FrameGraphNode
	{
		uint32_t					RefCount = 0u;

		RenderPassHandle			RenderPass;
		FrameGraphHandle			FrameBufferHandle;

		D_CORE::Ref<D_RENDERER::RenderPass>	GraphRenderPass;

		D_CONTAINERS::DVector<FrameGraphResourceHandle>	Inputs;
		D_CONTAINERS::DVector<FrameGraphResourceHandle>	Outputs;

		D_CONTAINERS::DVector<FrameGraphNodeHandle>	Edges;

		bool						Enable = true;

		D_CORE::StringId			Name = ""_SId;
	};

	struct FrameGraphRenderPassCache
	{
		void						Initialize(D_MEMORY::Allocator* allocator);
		void						Shutdown();

		D_CONTAINERS::DFlatMap<uint64_t, RenderPass> RenderPassMap;
	};

	struct FrameGraphResourceCache
	{
		void						Initialize(D_MEMORY::Allocator* allocator);
		void						Shutdown();

		D_MEMORY::TypedObjectPool<FrameGraphResource>	Resources;
	};

	struct FrameGraphNodeCache
	{
		void						Initialize(D_MEMORY::Allocator* allocator);
		void						Shutdown();

		D_MEMORY::ObjectPool		Nodes;
	};

	class FrameGraphBuilder
	{
	public:
		void						Initialize();
		void						Shutdown();

		void						RegisterRenderPass(D_CORE::StringId const& name, RenderPass* renderPass);

		FrameGraphResourceHandle	CreateNodeOutput(FrameGraphResourceOutputCreation const& desc, FrameGraphNodeHandle producer);
		FrameGraphResourceHandle	CreateNodeInput(FrameGraphResourceInputCreation const& desc);
		FrameGraphNodeHandle		CreateNode(FrameGraphNodeCreation const& node);

		FrameGraphNode* GetNode(D_CORE::StringId const& name);
		FrameGraphNode* AccessNode(FrameGraphNodeHandle handle);

		FrameGraphResource* GetResouce(D_CORE::StringId const& name);
		FrameGraphResource* AccessResource(FrameGraphResourceHandle handle);

	private:
		FrameGraphResourceCache		mResouceCache;
		FrameGraphNodeCache			mNodeCache;
		FrameGraphRenderPassCache	mRenderPassCache;

		D_MEMORY::Allocator* mAllocator;

	public:
		static constexpr uint32_t	MaxRenderPassCount = 256u;
		static constexpr uint32_t	MaxResourceCount = 1024u;
		static constexpr uint32_t	MaxNodesCount = 1024u;
	};

	class FrameGraph
	{
	public:
		void						Initialize(FrameGraphBuilder* builder, std::shared_ptr<RenderPassManager> passManager);
		void						Shutdown();

		void						Parse(Darius::Core::Filesystem::Path const& path, D_MEMORY::StackAllocator* tempAllocator);

		// Each frame the graph is rebuilt so that it is possible to enable what nodes we are interested in
		void						Reset();
		void						EnableRenderPass(D_CORE::StringId const& passName);
		void						DisableRenderPass(D_CORE::StringId const& passName);
		void						Compile();
		void						Render(Darius::Graphics::CommandContext& context);
		void						OnResize(uint32_t width, uint32_t height);

		FrameGraphNode*				GetNode(D_CORE::StringId const& nodeName);
		FrameGraphNode*				AccessNode(FrameGraphNodeHandle handle);

		FrameGraphResource*			GetResource(D_CORE::StringId const& resouceName);
		FrameGraphResource*			AccessResource(FrameGraphResourceHandle handle);

		// In case we need to add a pass on the run
		void						AddNode(FrameGraphNodeCreation const& node);


	private:
		// Nodes sorted in topological sort
		D_CONTAINERS::DVector<FrameGraphNodeHandle> mNodes;
		D_CONTAINERS::DVector<FrameGraphNodeHandle> mAllNodes;

		FrameGraphBuilder* mBuilder;
		std::unique_ptr < D_MEMORY::Allocator> mAllocator;

		D_MEMORY::LinearAllocator* mLocalAllocator;

		std::shared_ptr<RenderPassManager>	mRenderPassManager;
		D_CORE::StringId			mName = ""_SId;


		friend void ComputeEdges(FrameGraph* frameGraph, FrameGraphNode* node, uint32_t nodeIndex);
	};
}
