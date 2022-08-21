#pragma once

#include "Window.hpp"

#include <Core/Containers/Vector.hpp>
#include <Scene/Scene.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/FrameResource.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Renderer/GraphicsUtils/Buffers/ColorBuffer.hpp>
#include <Renderer/GraphicsUtils/Buffers/DepthBuffer.hpp>
#include <Renderer/GraphicsUtils/Memory/DescriptorHeap.hpp>

#include <iostream>

using namespace D_CONTAINERS;

namespace Darius::Editor::Gui::Windows
{
	class SceneWindow : public Darius::Editor::Gui::Windows::Window
	{
	public:
		SceneWindow();
		~SceneWindow();

		SceneWindow(SceneWindow const& other) = delete;

		virtual inline std::string const GetName() override { return "Scene"; }
		virtual void Render(D_GRAPHICS::GraphicsContext& context) override;
		virtual void DrawGUI() override;
		virtual void Update(float dt) override;

	private:
		void CreateBuffers();
		void CreateGrid(DVector<D_RENDERER_FRAME_RESOUCE::RenderItem>& items);

		DVector<D_RENDERER_FRAME_RESOUCE::RenderItem> GetRenderItems();

		D_RENDERER_FRAME_RESOUCE::GlobalConstants GetGlobalConstants();

		D_MATH_CAMERA::Camera						mCamera;

		D_GRAPHICS_BUFFERS::ColorBuffer				mSceneTexture;
		D_GRAPHICS_BUFFERS::DepthBuffer				mSceneDepth;
		D_GRAPHICS_MEMORY::DescriptorHandle			mTextureHandle;

		Ref<MeshResource>							mLineMeshResource;
		Ref<MaterialResource>						mLineMeshMaterial;

		D_GRAPHICS_BUFFERS::UploadBuffer			mLineConstantsCPU;
		ByteAddressBuffer							mLineConstantsGPU;

	};

}
