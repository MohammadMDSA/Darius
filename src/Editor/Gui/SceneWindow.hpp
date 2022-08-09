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
		inline DVector<D_RENDERER_FRAME_RESOUCE::RenderItem> GetRenderItems();

		D_RENDERER_FRAME_RESOUCE::GlobalConstants GetGlobalConstants();

		D_MATH_CAMERA::Camera						mCamera;

		D_GRAPHICS_BUFFERS::ColorBuffer				mSceneTexture;
		D_GRAPHICS_BUFFERS::DepthBuffer				mSceneDepth;
		D_GRAPHICS_MEMORY::DescriptorHandle			mTextureHandle;

	};

	inline DVector<D_RENDERER_FRAME_RESOUCE::RenderItem> SceneWindow::GetRenderItems()
	{
		// TODO: WRITE IT BETTER - I will when I implement components

		// Update CBs
		DVector<RenderItem> items;
		const auto gos = D_SCENE::GetGameObjects();
		auto cam = D_CAMERA_MANAGER::GetActiveCamera();
		auto frustum = cam->GetViewSpaceFrustum();
		for (auto itr = gos->begin(); itr != gos->end(); itr++)
		{
			auto go = *itr;
			
			// Is it active or renderable
			if (!go->CanRender())
				continue;

			// Is it in our frustum
			auto bsp = go->mTransform * go->GetBounds();
			auto vsp = BoundingSphere(Vector3(cam->GetViewMatrix() * bsp.GetCenter()), bsp.GetRadius());
			if (!frustum.IntersectSphere(vsp))
				continue;

			// Add it to render list
			auto item = go->GetRenderItem();
			items.push_back(item);
		}

		// TODO: use logging - Sure but logging is not implemented yet!
		std::cout << "Number of render items: " << items.size() << std::endl;
		return items;
	}
}
