#include "Renderer/pch.hpp"
#include "RenderPassManager.hpp"

#include "Renderer/FrameGraph/RenderPass.hpp"

#include "Renderer/Rasterization/Passes/RasterizationSkyboxPass.hpp"

namespace Darius::Renderer
{
	void RenderPassManager::Initialize()
	{
		mLastHandleIndex = 0u;

		// All passes mush be registered here manually
		RegisterRenderPass<D_RENDERER_RAST::RasterizationSkyboxPass>();
	}

	RenderPassManager::~RenderPassManager()
	{
		this->Shutdown();
	}

	void RenderPassManager::Shutdown()
	{
		mRenderPasses.clear();
		mRenderPassFactories.clear();
	}

	RenderPassHandle RenderPassManager::CreateRenderPass(D_CORE::StringId const& passName)
	{
		auto search = mRenderPassFactories.find(passName);
		if (search == mRenderPassFactories.end())
			return InvalidRenderPassHandle;

		auto newPass = search->second->CreateRenderPass();

		mLastHandleIndex++;

		RenderPassHandle newHandle = { mLastHandleIndex };

		mRenderPasses[newHandle] = std::unique_ptr<RenderPass>(newPass);
		return newHandle;
	}

	RenderPass* RenderPassManager::GetRenderPass(RenderPassHandle handle) const
	{
		auto search = mRenderPasses.find(handle);
		if (search == mRenderPasses.end())
			return nullptr;

		return search->second.get();
	}

	void RenderPassManager::ClearPasses()
	{
		mRenderPasses.clear();
	}

	bool RenderPassManager::DeleteRenderPass(RenderPassHandle handle)
	{
		return mRenderPasses.erase(handle);
	}
}