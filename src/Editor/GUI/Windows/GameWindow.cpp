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

using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_RENDERER;
using namespace D_RENDERER_FRAME_RESOURCE;
using namespace DirectX;

namespace Darius::Editor::Gui::Windows
{
	GameWindow::GameWindow(D_SERIALIZATION::Json const& config) :
		Window(config)
	{
		CreateBuffers();
		mTextureHandle = D_RENDERER::AllocateUiTexture();

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
		
		auto camera = D_CAMERA_MANAGER::GetActiveCamera();

		if (!camera.IsValid() || !camera->IsActive() || !UpdateGlobalConstants(mSceneGlobals))
		{
			// Clearing depth
			context.TransitionResource(mSceneDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
			context.ClearDepth(mSceneDepth);

			// Clear scene color
			context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			context.ClearColor(mSceneTexture);
			return;
		}
		
		D_MATH_CAMERA::Camera const& c = camera.Get()->GetCamera();

		D_RENDERER::SceneRenderContext rc = { mSceneDepth, mSceneTexture, context, c, mSceneGlobals, true};
		D_RENDERER::Render(rc, nullptr,
			[&](D_RENDERER::MeshSorter& sorter)
			{
				// Add debug draw items
				if (true /*mDrawDebug*/)
				{
					D_DEBUG_DRAW::GetRenderItems(sorter);
					sorter.Sort();
					sorter.RenderMeshes(MeshSorter::kTransparent, context, mSceneGlobals);
				}
			});
		D_RENDERER_DEVICE::GetDevice()->CopyDescriptorsSimple(1, mTextureHandle, mSceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	}

	void GameWindow::DrawGUI()
	{
		auto cam = D_CAMERA_MANAGER::GetActiveCamera();
		if (!cam.IsValid() || !cam->IsActive())
		{
			ImGui::Text("No ACTIVE CAMERA IN SCENE!");
		}
		else
			ImGui::Image((ImTextureID)mTextureHandle.GetGpuPtr(), ImVec2(mWidth, mHeight));

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
		mSceneTexture.Create(L"Game Scene Texture", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, D_RENDERER_DEVICE::GetBackBufferFormat());
		mSceneDepth.Create(L"Game Scene DepthStencil", (UINT)mBufferWidth, (UINT)mBufferHeight, D_RENDERER_DEVICE::GetDepthBufferFormat());
	}

	bool GameWindow::UpdateGlobalConstants(D_RENDERER_FRAME_RESOURCE::GlobalConstants& globals)
	{
		Matrix4 temp;

		float width, height;
		D_CAMERA_MANAGER::GetViewportDimansion(width, height);

		auto time = *D_TIME::GetStepTimer();

		auto cameraP = D_CAMERA_MANAGER::GetActiveCamera();
		if (!cameraP.IsValid() || !cameraP->IsActive())
			return false;

		D_MATH_CAMERA::Camera const& camera = cameraP->GetCamera();

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
