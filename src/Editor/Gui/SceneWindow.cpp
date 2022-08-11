#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "SceneWindow.hpp"
#include "Editor/Camera.hpp"
#include "Editor/EditorContext.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Core/Input.hpp>
#include <Renderer/Renderer.hpp>

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>

namespace Darius::Editor::Gui::Windows
{
	SceneWindow::SceneWindow()
	{
		CreateBuffers();
		mTextureHandle = D_RENDERER::GetRenderResourceHandle(1);

		mCamera.SetFOV(XM_PI / 3);
		mCamera.SetZRange(0.01f, 1000.f);
		mCamera.SetPosition(Vector3(-5.f, 0.f, 0));
		mCamera.SetLookDirection(Vector3::Right(), Vector3::Up());
		mCamera.ReverseZ(false);

		D_CAMERA_MANAGER::SetActiveCamera(&mCamera);

	}

	SceneWindow::~SceneWindow()
	{
		mSceneDepth.Destroy();
		mSceneTexture.Destroy();
	}

	D_RENDERER_FRAME_RESOUCE::GlobalConstants SceneWindow::GetGlobalConstants()
	{
		D_RENDERER_FRAME_RESOUCE::GlobalConstants globals;

		auto view = mCamera.GetViewMatrix();
		auto proj = mCamera.GetProjMatrix();

		auto viewProj = mCamera.GetViewProjMatrix();
		auto invView = Matrix4::Inverse(view);
		auto invProj = Matrix4::Inverse(proj);
		auto invViewProj = Matrix4::Inverse(viewProj);

		float width, height;
		D_CAMERA_MANAGER::GetViewportDimansion(width, height);

		auto time = *D_TIME::GetStepTimer();

		globals.View = view;
		globals.InvView = invView;
		globals.Proj = proj;
		globals.InvProj = invProj;
		globals.ViewProj = viewProj;
		globals.InvViewProj = invViewProj;
		globals.CameraPos = mCamera.GetPosition();
		globals.RenderTargetSize = XMFLOAT2(width, height);
		globals.InvRenderTargetSize = XMFLOAT2(1.f / width, 1.f / height);
		globals.NearZ = mCamera.GetNearClip();
		globals.FarZ = mCamera.GetFarClip();
		globals.TotalTime = (float)time.GetTotalSeconds();
		globals.DeltaTime = (float)time.GetElapsedSeconds();
		globals.AmbientLight = { 0.5f, 0.5f, 0.5f, 1.0f };


		globals.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		globals.Lights[0].Color = { 0.6f, 0.6f, 0.6f };
		globals.Lights[0].Color = { 0.6f, 0.6f, 0.6f };
		globals.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		globals.Lights[1].Color = { 0.3f, 0.3f, 0.3f };
		globals.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		globals.Lights[2].Color = { 0.15f, 0.15f, 0.15f };

		return globals;
	}

	void SceneWindow::Render(D_GRAPHICS::GraphicsContext& context)
	{
		context.SetPipelineState(D_RENDERER::Psos["opaque"]);

		// Prepare the command list to render a new frame.
		context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		PIXBeginEvent(PIX_COLOR_DEFAULT, "Clear Scene Buffers");

		// Clear the views.
		auto const rtvDescriptor = mSceneTexture.GetRTV();
		auto const dsvDescriptor = mSceneDepth.GetDSV();

		// Set the viewport and scissor rect.
		auto viewport = CD3DX12_VIEWPORT(0.f, 0.f, mWidth, mHeight, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissorRect = CD3DX12_RECT(0l, 0l, (long)(mWidth), (long)(mHeight));

		context.ClearColor(mSceneTexture, &scissorRect);
		context.ClearDepth(mSceneDepth);
		context.SetRenderTarget(rtvDescriptor, dsvDescriptor);
		context.SetViewportAndScissor(viewport, scissorRect);

		PIXEndEvent();

		auto globals = GetGlobalConstants();
		D_RENDERER::RenderMeshes(context, GetRenderItems(), globals);

		context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		D_RENDERER_DEVICE::GetDevice()->CopyDescriptorsSimple(1, mTextureHandle, mSceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	void SceneWindow::DrawGUI()
	{
		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();

		ImGui::Image((ImTextureID)mTextureHandle.GetGpuPtr(), ImVec2(mWidth, mHeight));
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(mPosX + min.x, mPosY + min.y, mWidth, mHeight);
		auto view = mCamera.GetViewMatrix();
		auto proj = mCamera.GetProjMatrix();

		auto selectedObj = D_EDITOR_CONTEXT::GetSelectedGameObject();

		if (selectedObj)
		{
			auto world = selectedObj->mTransform.GetWorld();
			if (ImGuizmo::Manipulate((float*)&view, (float*)&proj, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, (float*)&world))
				selectedObj->mTransform = D_MATH::Transform(D_MATH::Matrix4(world));
		}
	}

	void SceneWindow::Update(float dt)
	{
		if (D_CAMERA_MANAGER::SetViewportDimansion(mWidth, mHeight))
		{
			CreateBuffers();
		}

		static auto fc = FlyingFPSCamera(mCamera, Vector3::Up());

		if (D_MOUSE::GetButton(D_MOUSE::Keys::Right) && mHovered)
			fc.Update(dt);
		else
			mCamera.Update();

	}

	void SceneWindow::CreateBuffers()
	{
		mSceneTexture.Create(L"Scene Texture", (UINT)mWidth, (UINT)mHeight, 1, D_RENDERER_DEVICE::GetBackBufferFormat());
		mSceneDepth.Create(L"Scene DepthStencil", (UINT)mWidth, (UINT)mHeight, D_RENDERER_DEVICE::GetDepthBufferFormat());
	}

	void SceneWindow::CreateGrid(DVector<D_RENDERER_FRAME_RESOUCE::RenderItem>&)
	{
		
	}

	DVector<D_RENDERER_FRAME_RESOUCE::RenderItem> SceneWindow::GetRenderItems()
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

		D_LOG_INFO("Number of render items: " + std::to_string(items.size()));

		CreateGrid(items);

		return items;
	}
}
