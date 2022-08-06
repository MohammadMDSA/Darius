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
		// TODO: WRITE IT BETTER

		// Update CBs
		DVector<RenderItem> items;
		const auto gos = D_SCENE::GetGameObjects();
		UINT index = 0;
		auto cam = D_CAMERA_MANAGER::GetActiveCamera();
		auto frustum = cam->GetViewSpaceFrustum();
		for (auto itr = gos->begin(); itr != gos->end(); itr++)
		{
			auto go = *itr;
			if (!go->mMeshResource.IsValid())
				continue;
			auto item = go->GetRenderItem();
			auto bsp = go->mTransform * go->mMeshResource.Get()->Get()->mBoundSp;
			auto vsp = BoundingSphere(Vector3(cam->GetViewMatrix() * bsp.GetCenter()), bsp.GetRadius());
			if (!frustum.IntersectSphere(vsp))
				continue;

			item.ObjCBIndex = index++;
			items.push_back(item);
		}
		return items;
	}
}
