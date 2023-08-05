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

		// Main Buffers
		D_GRAPHICS_BUFFERS::ColorBuffer				mSceneTexture;
		D_GRAPHICS_BUFFERS::DepthBuffer				mSceneDepth;
		D_GRAPHICS_BUFFERS::ColorBuffer				mSceneNormals;

		// TAA Buffers
		D_GRAPHICS_BUFFERS::ColorBuffer				mTemporalColor[2];
		D_GRAPHICS_BUFFERS::ColorBuffer				mVelocityBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer				mLinearDepth[2];

		// Post Processing Buffers
		// HDR ToneMapping
		D_GRAPHICS_BUFFERS::StructuredBuffer		mExposureBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer				mLumaBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer				mLumaLR;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer		mHistogramBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer				mPostEffectsBuffer;
		// Bloom
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV1[2];
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV2[2];
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV3[2];
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV4[2];
		D_GRAPHICS_BUFFERS::ColorBuffer             BloomUAV5[2];

		// Ambient Occlusion Buffers
		D_GRAPHICS_BUFFERS::ColorBuffer				mSSAOFullScreen;
		D_GRAPHICS_BUFFERS::ColorBuffer				mDepthDownsize1;
		D_GRAPHICS_BUFFERS::ColorBuffer				mDepthDownsize2;
		D_GRAPHICS_BUFFERS::ColorBuffer				mDepthDownsize3;
		D_GRAPHICS_BUFFERS::ColorBuffer				mDepthDownsize4;
		D_GRAPHICS_BUFFERS::ColorBuffer				mDepthTiled1;
		D_GRAPHICS_BUFFERS::ColorBuffer				mDepthTiled2;
		D_GRAPHICS_BUFFERS::ColorBuffer				mDepthTiled3;
		D_GRAPHICS_BUFFERS::ColorBuffer				mDepthTiled4;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOMerged1;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOMerged2;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOMerged3;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOMerged4;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOSmooth1;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOSmooth2;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOSmooth3;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOHighQuality1;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOHighQuality2;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOHighQuality3;
		D_GRAPHICS_BUFFERS::ColorBuffer				mAOHighQuality4;

		D_GRAPHICS_MEMORY::DescriptorHandle			mTextureHandle;

		D_RENDERER::GlobalConstants					mSceneGlobals;

		float										mBufferWidth;
		float										mBufferHeight;
 	};
}
