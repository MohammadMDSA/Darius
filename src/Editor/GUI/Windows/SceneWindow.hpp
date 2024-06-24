#pragma once

#include "Window.hpp"
#include "Editor/Camera.hpp"
#include "Editor/GUI/PostProcessing/GuiPostProcessing.hpp"

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

		D_MATH::Vector3 SuggestSpawnPosition(float dist) const;
		D_MATH::Vector3 SuggestSpawnPositionOnYPlane() const;

	private:
		void CreateBuffers();

		void CreateGrid(D_CONTAINERS::DVector<D_RENDERER::RenderItem>& items, int count);
		void CalcGridLineConstants(D_CONTAINERS::DVector<D_RENDERER::MeshConstants>& constants, int count);

		void UpdateGlobalConstants(D_RENDERER::GlobalConstants& globals) const;

		bool SelectPickedGameObject();

		D_MATH_CAMERA::Camera						mCamera;
		D_EDITOR::FlyingFPSCamera					mFlyingCam;
		D_EDITOR::OrbitCamera						mOrbitCam;

		UINT										mManipulateOperation;
		UINT										mManipulateMode;

		D_GRAPHICS_BUFFERS::ColorBuffer				mPostProcessedSceneTexture;

		D_GRAPHICS_BUFFERS::ColorBuffer				mPickerColor;
		D_GRAPHICS_BUFFERS::DepthBuffer				mPickerDepth;

		D_RENDERER::RenderViewBuffers				mRenderBuffers;

		D_GRAPHICS_MEMORY::DescriptorHandle			mTextureHandle;

		D_RESOURCE::ResourceRef<D_RENDERER::BatchResource> mLineMeshResource;
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource> mSkyboxDiff;
		D_RESOURCE::ResourceRef<D_RENDERER::TextureResource> mSkyboxSpec;

		D_GRAPHICS_BUFFERS::ByteAddressBuffer		mLineConstantsGPU;

		D_CONTAINERS::DVector<D_RENDERER::RenderItem> mWindowRenderItems;
		D_RENDERER::GlobalConstants					mSceneGlobals;

		D_GRAPHICS_BUFFERS::ReadbackBuffer			mPickerReadback;

		D_EDITOR::Gui::PostProcessing::GuiPostProcessing mGuiPostProcess;

		float										mMouseWheelPerspectiveSensitivity;

		float										mBufferWidth;
		float										mBufferHeight;

		D_MATH::Vector2								mMouseSceneTexturePos;

		static constexpr UINT8						sSelectedGameObjectStencilValue = 255;

		int											mDrawGrid : 1;
		int											mDrawSkybox : 1;
		int											mMovingCam : 1;
		int											mDrawDebug : 1;
		int											mForceWireframe : 1;
		int											mCustomDepthApplied : 1;
		int											mDragSpawned : 1;
	};

}
