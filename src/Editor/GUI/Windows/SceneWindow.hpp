#pragma once

#include "Window.hpp"
#include "Editor/Camera.hpp"

#include <Core/Containers/Vector.hpp>
#include <Core/RefCounting/Ref.hpp>
#include <Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp>
#include <Graphics/GraphicsUtils/Buffers/DepthBuffer.hpp>
#include <Graphics/GraphicsUtils/Memory/DescriptorHeap.hpp>
#include <Renderer/RendererManager.hpp>
#include <Renderer/RendererCommon.hpp>
#include <Renderer/Resources/BatchResource.hpp>
#include <Renderer/Resources/TextureResource.hpp>
#include <Scene/Scene.hpp>

#include <iostream>

namespace Darius::Editor::Gui::Windows
{
	class SceneWindow : public Darius::Editor::Gui::Windows::Window
	{
		D_CH_EDITOR_WINDOW_BODY(SceneWindow, "Scene");
	public:
		SceneWindow(SceneWindow const& other) = delete;

		virtual void Render() override;
		virtual void DrawGUI() override;
		virtual void Update(float dt) override;

	private:
		void CreateBuffers();

		void CreateGrid(D_CONTAINERS::DVector<D_RENDERER::RenderItem>& items, int count);
		void CalcGridLineConstants(D_CONTAINERS::DVector<D_RENDERER::MeshConstants>& constants, int count);

		void UpdateGlobalConstants(D_RENDERER::GlobalConstants& globals) const;

		D_MATH_CAMERA::Camera						mCamera;
		D_EDITOR::FlyingFPSCamera					mFlyingCam;
		D_EDITOR::OrbitCamera						mOrbitCam;

		UINT										mManipulateOperation;
		UINT										mManipulateMode;

		// Main Buffers
		D_GRAPHICS_BUFFERS::ColorBuffer				mSceneTexture;
		D_GRAPHICS_BUFFERS::DepthBuffer				mSceneDepth;
		D_GRAPHICS_BUFFERS::DepthBuffer				mCustomDepth;
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

		D_GRAPHICS_BUFFERS::ColorBuffer				mWorldPos;
		D_GRAPHICS_BUFFERS::ColorBuffer				mNormalDepth;

		D_GRAPHICS_MEMORY::DescriptorHandle			mTextureHandle;

		D_RESOURCE::ResourceRef<D_RENDERER::BatchResource> mLineMeshResource;
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource> mSkyboxDiff;
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource> mSkyboxSpec;

		D_GRAPHICS_BUFFERS::ByteAddressBuffer		mLineConstantsGPU;

		D_CONTAINERS::DVector<D_RENDERER::RenderItem> mWindowRenderItems;
		D_RENDERER::GlobalConstants	mSceneGlobals;

		float										mMouseWheelPerspectiveSensitivity;

		float										mBufferWidth;
		float										mBufferHeight;

		UINT8										mSelectedGameObjectStencilValue = 255;

		bool										mDrawGrid;
		bool										mDrawSkybox;
		bool										mMovingCam;
		bool										mDrawDebug;
		bool										mForceWireframe;
		bool										mCustomDepthApplied;
	};

}
