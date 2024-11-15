#pragma once

#include <Core/Containers/Map.hpp>
#include <Core/StringId.hpp>

#ifndef D_RENDERER
#define D_RENDERER Darius::Renderer
#endif // !D_RENDERER

namespace Darius::Renderer
{
	class RenderPass;
	class RenderPassFactory;

	struct RenderPassHandle
	{
		uint32_t					Index;

		bool IsValid() const;
	};

	RenderPassHandle InvalidRenderPassHandle = { UINT_MAX };
	inline bool RenderPassHandle::IsValid() const { return Index != InvalidRenderPassHandle.Index; }

	class RenderPassManager
	{
	public:
		~RenderPassManager();

		void Initialize();
		void Shutdown();

		void ClearPasses();

		RenderPassHandle	CreateRenderPass(D_CORE::StringId const& passName);

		RenderPass*			GetRenderPass(RenderPassHandle handle) const;
		bool				DeleteRenderPass(RenderPassHandle handle);

	private:

		template<class RENDER_PASS>
		void RegisterRenderPass()
		{
			auto factory = new RENDER_PASS::Factory();
			mRenderPassFactories.emplace(RENDER_PASS::GetPassNameStatic(), factory);
		}

		D_CONTAINERS::DUnorderedMap<RenderPassHandle, std::unique_ptr<RenderPass>> mRenderPasses;
		D_CONTAINERS::DUnorderedMap<D_CORE::StringId, std::unique_ptr<RenderPassFactory>> mRenderPassFactories;

		// TODO: Better index allocation
		uint32_t			mLastHandleIndex;
	};
}