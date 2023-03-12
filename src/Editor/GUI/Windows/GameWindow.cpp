#include "Editor/pch.hpp"
#include "GameWindow.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Renderer/Components/MeshRendererComponent.hpp>
#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>
#include <Renderer/Light/LightManager.hpp>
#include <Renderer/RenderDeviceManager.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

namespace Darius::Editor::Gui::Windows
{
	GameWindow::GameWindow(D_SERIALIZATION::Json const& config) :
		Window(config)
	{
		CreateBuffers();
		mTextureHandle = D_RENDERER::AllocateUiTexture();
		D_RENDERER_DEVICE::GetDevice()->CopyDescriptorsSimple(1, mTextureHandle, mSceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


		// Window padding
		mPadding[0] = mPadding[1] = 0.f;

	}

	GameWindow::~GameWindow()
	{
		mSceneDepth.Destroy();
		mSceneTexture.Destroy();
	}

	void GameWindow::Render(D_GRAPHICS::GraphicsContext& context)
	{
		// Clearing depth
		context.TransitionResource(mSceneDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		context.ClearDepth(mSceneDepth);

		auto camera = D_CAMERA_MANAGER::GetActiveCamera();

		if (!camera || !UpdateGlobalConstants(mSceneGlobals))
			return;

		// Setting up sorter
		auto viewPort = CD3DX12_VIEWPORT(0.f, 0.f, mWidth, mHeight, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissor = CD3DX12_RECT(0l, 0l, (long)mWidth, (long)mHeight);
		MeshSorter sorter(MeshSorter::kDefault);
		sorter.SetCamera(*camera);
		sorter.SetViewport(viewPort);
		sorter.SetScissor(scissor);
		sorter.SetDepthStencilTarget(mSceneDepth);
		sorter.AddRenderTarget(mSceneTexture);

		// Add meshes to sorter
		AddSceneRenderItems(sorter, camera);

		{
			// Creating shadows

			DVector<RenderItem> shadowRenderItems;
			PopulateShadowRenderItems(shadowRenderItems);

			D_LIGHT::RenderShadows(shadowRenderItems);
		}

		sorter.Sort();

		// Clearing scene color texture
		context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		context.ClearColor(mSceneTexture);


		D_RENDERER::DrawSkybox(context, *camera, mSceneTexture, mSceneDepth, viewPort, scissor);

		sorter.RenderMeshes(MeshSorter::kTransparent, context, mSceneGlobals);

		// Add debug draw items
		MeshSorter debugDrawSorter(sorter);
		D_DEBUG_DRAW::GetRenderItems(debugDrawSorter);
		debugDrawSorter.Sort();
		debugDrawSorter.RenderMeshes(MeshSorter::kTransparent, context, mSceneGlobals);

		context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

	}

	void GameWindow::DrawGUI()
	{
		auto cam = D_CAMERA_MANAGER::GetActiveCamera();
		if (!cam)
		{
			ImGui::Text("No ACTIVE CAMERA IN SCENE!");
		}
	}

	void GameWindow::Update(float)
	{
		if (D_CAMERA_MANAGER::SetViewportDimansion(mWidth, mHeight))
		{
			CreateBuffers();
		}


	}

	void GameWindow::CreateBuffers()
	{
		mBufferWidth = mWidth;
		mBufferHeight = mHeight;
		mSceneTexture.Create(L"Scene Texture", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, D_RENDERER_DEVICE::GetBackBufferFormat());
		mSceneDepth.Create(L"Scene DepthStencil", (UINT)mBufferWidth, (UINT)mBufferHeight, D_RENDERER_DEVICE::GetDepthBufferFormat());
	}

	bool GameWindow::UpdateGlobalConstants(D_RENDERER_FRAME_RESOURCE::GlobalConstants& globals)
	{
		Matrix4 temp;

		float width, height;
		D_CAMERA_MANAGER::GetViewportDimansion(width, height);

		auto time = *D_TIME::GetStepTimer();

		auto cameraP = D_CAMERA_MANAGER::GetActiveCamera();
		if (!cameraP)
			return false;

		auto& camera = *cameraP;

		temp = camera.GetViewMatrix(); // View
		globals.View = temp;
		globals.InvView = Matrix4::Inverse(temp);

		temp = camera.GetProjMatrix(); // Proj
		globals.Proj = temp;
		globals.InvProj = Matrix4::Inverse(temp);

		temp = camera.GetViewProjMatrix(); // View proj
		globals.ViewProj = temp;
		globals.InvViewProj = Matrix4::Inverse(temp);

		globals.CameraPos = camera.GetPosition();
		globals.RenderTargetSize = XMFLOAT2(width, height);
		globals.InvRenderTargetSize = XMFLOAT2(1.f / width, 1.f / height);
		globals.NearZ = camera.GetNearClip();
		globals.FarZ = camera.GetFarClip();
		globals.TotalTime = (float)time.GetTotalSeconds();
		globals.DeltaTime = (float)time.GetElapsedSeconds();
		globals.AmbientLight = { 0.1f, 0.1f, 0.1f, 1.0f };

		return true;
	}

	void GameWindow::PopulateShadowRenderItems(D_CONTAINERS::DVector<RenderItem>& items) const
	{
		auto& worldReg = D_WORLD::GetRegistry();

		// Iterating over meshes
		worldReg.each([&](D_GRAPHICS::MeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				if (!meshComp.GetCastsShadow())
					return;

				items.push_back(meshComp.GetRenderItem());
			});

		// Iterating over meshes
		worldReg.each([&](D_GRAPHICS::SkeletalMeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				if (!meshComp.GetCastsShadow())
					return;

				items.push_back(meshComp.GetRenderItem());
			});
	}

	void GameWindow::AddSceneRenderItems(D_RENDERER::MeshSorter& sorter, D_MATH::Camera::Camera* cam) const
	{
		auto& worldReg = D_WORLD::GetRegistry();

		auto frustum = cam->GetViewSpaceFrustum();

		// Iterating over meshes
		worldReg.each([&](D_GRAPHICS::MeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				// Is it in our frustum
				auto sphereWorldSpace = meshComp.GetGameObject()->GetTransform() * meshComp.GetBounds();
				auto sphereViewSpace = BoundingSphere(Vector3(cam->GetViewMatrix() * sphereWorldSpace.GetCenter()), sphereWorldSpace.GetRadius());
				if (!frustum.IntersectSphere(sphereViewSpace))
					return;

				auto distance = -sphereViewSpace.GetCenter().GetZ() - sphereViewSpace.GetRadius();
				sorter.AddMesh(meshComp.GetRenderItem(), distance);
			});

		// Iterating over meshes
		worldReg.each([&](D_GRAPHICS::SkeletalMeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				// Is it in our frustum
				auto sphereWorldSpace = meshComp.GetGameObject()->GetTransform() * meshComp.GetBounds();
				auto sphereViewSpace = BoundingSphere(Vector3(cam->GetViewMatrix() * sphereWorldSpace.GetCenter()), sphereWorldSpace.GetRadius());
				if (!frustum.IntersectSphere(sphereViewSpace))
					return;

				auto distance = -sphereViewSpace.GetCenter().GetZ() - sphereViewSpace.GetRadius();
				sorter.AddMesh(meshComp.GetRenderItem(), distance);
			});

		//D_LOG_DEBUG("Number of render items: " << sorter.CountObjects());
	}
}
