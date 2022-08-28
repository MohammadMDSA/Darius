#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "SceneWindow.hpp"
#include "Editor/Camera.hpp"
#include "Editor/EditorContext.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Core/Input.hpp>
#include <Renderer/Renderer.hpp>
#include <ResourceManager/ResourceManager.hpp>

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
		mCamera.SetPosition(Vector3(2.f, 2.f, 2.f));
		mCamera.SetLookDirection(Vector3(-2), Vector3::Up());
		mCamera.ReverseZ(false);

		D_CAMERA_MANAGER::SetActiveCamera(&mCamera);

		// Fetch line mesh resource
		auto lineHandle = D_RESOURCE::GetDefaultResource(D_RESOURCE::DefaultResource::LineMesh);
		mLineMeshResource = D_RESOURCE::GetResource<BatchResource>(lineHandle, this, L"Scene Window", "Editor Window");

		// Initializing grid gpu constants
		auto count = 50;
		auto total = 2 * (2 * count + 1);
		DVector<MeshConstants> consts;
		CalcGridLineConstants(consts, count);
		mLineConstantsGPU.Create(L"Scene Window Grid GPU Buffer", total, sizeof(MeshConstants), consts.data());

		CreateGrid(mWindowRenderItems, total);
	}

	SceneWindow::~SceneWindow()
	{
		mSceneDepth.Destroy();
		mSceneTexture.Destroy();
	}

	void SceneWindow::UpdateGlobalConstants(D_RENDERER_FRAME_RESOUCE::GlobalConstants& globals)
	{
		Matrix4 temp;

		float width, height;
		D_CAMERA_MANAGER::GetViewportDimansion(width, height);

		auto time = *D_TIME::GetStepTimer();

		temp = mCamera.GetViewMatrix(); // View
		globals.View = temp;
		globals.InvView = Matrix4::Inverse(temp);

		temp = mCamera.GetProjMatrix(); // Proj
		globals.Proj = temp;
		globals.InvProj = Matrix4::Inverse(temp);

		temp = mCamera.GetViewProjMatrix(); // View proj
		globals.ViewProj = temp;
		globals.InvViewProj = Matrix4::Inverse(temp);

		globals.CameraPos = mCamera.GetPosition();
		globals.RenderTargetSize = XMFLOAT2(width, height);
		globals.InvRenderTargetSize = XMFLOAT2(1.f / width, 1.f / height);
		globals.NearZ = mCamera.GetNearClip();
		globals.FarZ = mCamera.GetFarClip();
		globals.TotalTime = (float)time.GetTotalSeconds();
		globals.DeltaTime = (float)time.GetElapsedSeconds();
		globals.AmbientLight = { 0.5f, 0.5f, 0.5f, 1.0f };

		// Lights
		globals.Lights[0].Position = { 10.f, 10.f, 10.f };
		globals.Lights[0].Direction = { 0.57735f, -0.57735f, 0.57735f };
		globals.Lights[0].Color = { 0.6f, 0.6f, 0.6f };
		globals.Lights[1].Direction = { -0.57735f, -0.57735f, 0.57735f };
		globals.Lights[1].Color = { 0.3f, 0.3f, 0.3f };
		globals.Lights[2].Position = { 10.f, -10.f, 10.f };
		globals.Lights[2].Direction = { 0.0f, -0.707f, -0.707f };
		globals.Lights[2].Color = { 0.15f, 0.15f, 0.15f };

	}

	void SceneWindow::Render(D_GRAPHICS::GraphicsContext& context)
	{

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

		UpdateGlobalConstants(mSceneGlobals);

		// Updating render items
		UpdateSceneRenderItems(mMesheRenderItems);

		// Render scene
		context.SetPipelineState(D_RENDERER::GetPSO(PipelineStateTypes::Opaque));
		D_RENDERER::RenderMeshes(context, mMesheRenderItems, mSceneGlobals);

		// Render window batches
		context.SetPipelineState(D_RENDERER::GetPSO(PipelineStateTypes::Color));
		D_RENDERER::RenderBatchs(context, mWindowRenderItems, mSceneGlobals);

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
			auto transform = *selectedObj->GetTransform();
			auto world = transform.GetWorld();
			if (ImGuizmo::Manipulate((float*)&view, (float*)&proj, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, (float*)&world))
			{
				selectedObj->SetTransform(D_MATH::Transform(D_MATH::Matrix4(world)));
			}
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

	void SceneWindow::CalcGridLineConstants(DVector<MeshConstants>& constants, int count)
	{
		auto scale = Matrix4::MakeScale(count * 2);
		auto rot = Matrix4::MakeLookAt(Vector3(kZero), Vector3(-1.f, 0.f, 0.f), Vector3::Up());

		for (short i = 0; i <= count; i++)
		{
			// Along +x
			constants.push_back({ Matrix4::MakeTranslation(i, 0.f, count) * scale });

			// Along +z
			constants.push_back({ Matrix4::MakeTranslation(-count, 0.f, (float)i) * rot * scale });

			if (i == 0)
				continue;

			// Along -x
			constants.push_back({ Matrix4::MakeTranslation(-i, 0.f, count) * scale });

			// Along -z
			constants.push_back({ Matrix4::MakeTranslation(-count, 0.f, -i) * rot * scale });
		}
	}

	void SceneWindow::CreateGrid(DVector<D_RENDERER_FRAME_RESOUCE::RenderItem>& items, int count)
	{
		RenderItem item;
		const Mesh* mesh = mLineMeshResource->GetData();
		item.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		item.IndexCount = mesh->mDraw[0].IndexCount;
		item.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		item.Mesh = mesh;
		item.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		item.MeshCBV = mLineConstantsGPU.GetGpuVirtualAddress();

		for (int i = 0; i < count; i++)
		{
			if (i / 2 == 0)
				item.Color = { 1.f, 1.f, 1.f, 1.f };
			else if (((i - 2) / 4) % 10 == 9)
				item.Color = { 0.501f, 0.501f, 0.501f, 1.f };
			else
				item.Color = { 0.3f, 0.3f, 0.3f, 1.f };

			items.push_back(item);
			item.MeshCBV += sizeof(MeshConstants);
		}

	}

	void SceneWindow::UpdateSceneRenderItems(DVector<RenderItem>& items)
	{
		// TODO: WRITE IT BETTER - I will when I implement components

		items.clear();

		// Update CBs
		D_CONTAINERS::DVector<GameObject*> gos;
		D_WORLD::GetGameObjects(gos);

		auto cam = D_CAMERA_MANAGER::GetActiveCamera();
		auto frustum = cam->GetViewSpaceFrustum();
		for (auto itr = gos.begin(); itr != gos.end(); itr++)
		{
			auto go = *itr;

			// Is it active or renderable
			if (!go->CanRender())
				continue;

			// Is it in our frustum
			auto bsp = *go->GetTransform() * go->GetBounds();
			auto vsp = BoundingSphere(Vector3(cam->GetViewMatrix() * bsp.GetCenter()), bsp.GetRadius());
			if (!frustum.IntersectSphere(vsp))
				continue;

			// Add it to render list
			auto item = go->GetRenderItem();
			items.push_back(item);
		}

		//D_LOG_DEBUG("Number of render items: " + std::to_string(items.size()));

	}
}
