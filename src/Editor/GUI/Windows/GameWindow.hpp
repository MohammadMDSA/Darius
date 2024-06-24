#pragma once

#include "Window.hpp"

#include <Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/DepthBuffer.hpp>
#include <Renderer/RendererManager.hpp>
#include <Renderer/RendererCommon.hpp>

namespace Darius::Editor::Gui::Windows
{
	class GameWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY(GameWindow, "Game");

	public:
		
		virtual void				Render() override;
		virtual void				Update(float) override;
		virtual void				DrawGUI() override;

	private:
		bool						UpdateGlobalConstants(D_RENDERER::GlobalConstants& globals);
		void						CreateBuffers();

		D_RENDERER::RenderViewBuffers				mRenderBuffers;

		D_GRAPHICS_MEMORY::DescriptorHandle			mTextureHandle;

		D_RENDERER::GlobalConstants					mSceneGlobals;

		float										mBufferWidth;
		float										mBufferHeight;

		bool										mCustomDepthApplied;

		bool										mCapturingMouse;
 	};
}
