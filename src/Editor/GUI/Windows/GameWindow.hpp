#pragma once

#include "Window.hpp"

#include <Renderer/FrameResource.hpp>
#include <Renderer/GraphicsUtils/Buffers/ColorBuffer.hpp>
#include <Renderer/GraphicsUtils/Buffers/DepthBuffer.hpp>
#include <Renderer/Renderer.hpp>

namespace Darius::Editor::Gui::Windows
{
	class GameWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY(GameWindow, "Game");

	public:
		
		virtual void				Render(D_GRAPHICS::GraphicsContext&) override;
		virtual void				Update(float) override;
		virtual void				DrawGUI() override;

	private:
		bool						UpdateGlobalConstants(D_RENDERER_FRAME_RESOURCE::GlobalConstants& globals);
		void						CreateBuffers();
		void						AddSceneRenderItems(D_RENDERER::MeshSorter& sorter, D_MATH::Camera::Camera* cam) const;
		void						PopulateShadowRenderItems(D_CONTAINERS::DVector<RenderItem>& items) const;

		D_GRAPHICS_BUFFERS::ColorBuffer				mSceneTexture;
		D_GRAPHICS_BUFFERS::DepthBuffer				mSceneDepth;
		D_GRAPHICS_MEMORY::DescriptorHandle			mTextureHandle;

		D_RENDERER_FRAME_RESOURCE::GlobalConstants	mSceneGlobals;

		float										mBufferWidth;
		float										mBufferHeight;
 	};
}
