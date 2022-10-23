#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "SceneWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Core/Input.hpp>
#include <Debug/DebugDraw.hpp>
#include <Scene/Scene.hpp>
#include <Scene/EntityComponentSystem/Components/MeshRendererComponent.hpp>
#include <Scene/EntityComponentSystem/Components/SkeletalMeshRendererComponent.hpp>
#include <Renderer/Renderer.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

namespace Darius::Editor::Gui::Windows
{
	SceneWindow::SceneWindow() :
		mFlyingCam(mCamera, Vector3::Up()),
		mOrbitCam(mCamera, D_MATH_BOUNDS::BoundingSphere(0.f, 0.f, 0.f, 5.f), Vector3::Up()),
		mManipulateOperation(ImGuizmo::OPERATION::TRANSLATE),
		mManipulateMode(ImGuizmo::MODE::LOCAL),
		mDrawGrid(true),
		mDrawDebug(true)
	{
		CreateBuffers();
		mTextureHandle = D_RENDERER::GetUiTextureHandle(1);

		// Setup cameras
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

		// Window padding
		mPadding[0] = mPadding[1] = 0.f;
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
		globals.AmbientLight = { 0.1f, 0.1f, 0.1f, 1.0f };

	}

	void SceneWindow::Render(D_GRAPHICS::GraphicsContext& context)
	{

		UpdateGlobalConstants(mSceneGlobals);

		// Clearing depth
		context.TransitionResource(mSceneDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
		context.ClearDepth(mSceneDepth);

		// Setting up sorter
		MeshSorter sorter(MeshSorter::kDefault);
		sorter.SetCamera(mCamera);
		sorter.SetViewport(CD3DX12_VIEWPORT(0.f, 0.f, mWidth, mHeight, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH));
		sorter.SetScissor(CD3DX12_RECT(0l, 0l, (long)(mWidth), (long)(mHeight)));
		sorter.SetDepthStencilTarget(mSceneDepth);
		sorter.AddRenderTarget(mSceneTexture);

		// Add meshes to sorter
		AddSceneRenderItems(sorter);

		// Draw grid
		if (mDrawGrid)
			AddWindowRenderItems(sorter);


		// Add debug draw items
		if (mDrawDebug)
		{
			D_DEBUG_DRAW::GetRenderItems(sorter);
		}

		sorter.Sort();

		// Clearing scene color texture
		context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		context.ClearColor(mSceneTexture);

		sorter.RenderMeshes(MeshSorter::kOpaque, context, mSceneGlobals);
		sorter.RenderMeshes(MeshSorter::kTransparent, context, mSceneGlobals);

		context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		D_RENDERER_DEVICE::GetDevice()->CopyDescriptorsSimple(1, mTextureHandle, mSceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	void SceneWindow::DrawGUI()
	{
		D_PROFILING::ScopedTimer profiling(L"Scene Window Draw GUI");
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
			auto world = selectedObj->GetTransform().GetWorld();
			if (ImGuizmo::Manipulate((float*)&view, (float*)&proj, (ImGuizmo::OPERATION)mManipulateOperation, (ImGuizmo::MODE)mManipulateMode, (float*)&world))
			{
				selectedObj->SetTransform(world);
			}
		}

		// Drawing tool buttons
		ImGui::SetCursorPos({ 20.f, 30.f });
		{
			// Gizmo manipulation operation
			ImGuizmo::OPERATION operations[] =
			{
				ImGuizmo::OPERATION::TRANSLATE,
				ImGuizmo::OPERATION::ROTATE,
				ImGuizmo::OPERATION::SCALE
			};
			std::string OperationnNames[] =
			{
				ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT,
				ICON_FA_ROTATE,
				ICON_FA_RULER
			};
			for (size_t i = 0; i < 3; i++)
			{
				bool selected = (ImGuizmo::OPERATION)mManipulateOperation == operations[i];
				if (selected)
					ImGui::PushStyleColor(ImGuiCol_Button, { 0.26f, 0.59f, 1.f, 1.f });
				if (ImGui::Button(OperationnNames[i].c_str()))
				{
					mManipulateOperation = operations[i];
				}
				if (selected)
					ImGui::PopStyleColor();

				ImGui::SameLine();
			}

			ImGui::SameLine(150.f);

			// Gizmo manipulation mode
			ImGuizmo::MODE modes[] =
			{
				ImGuizmo::MODE::LOCAL,
				ImGuizmo::MODE::WORLD
			};
			std::string modeNames[] =
			{
				ICON_FA_GLOBE,
				ICON_FA_CUBE
			};

			for (size_t i = 0; i < 2; i++)
			{
				bool selected = (ImGuizmo::MODE)mManipulateMode == modes[i];
				if (selected)
					ImGui::PushStyleColor(ImGuiCol_Button, { 0.26f, 0.59f, 1.f, 1.f });
				if (ImGui::Button(modeNames[i].c_str()))
				{
					mManipulateMode = modes[i];
				}
				if (selected)
					ImGui::PopStyleColor();

				ImGui::SameLine();
			}

			ImGui::SameLine(230.f);

			// Grid settings
			{
				auto preDrawGrid = mDrawGrid;
				if (mDrawGrid)
					ImGui::PushStyleColor(ImGuiCol_Button, { 0.26f, 0.59f, 1.f, 1.f });
				if (ImGui::Button(ICON_FA_TABLE_CELLS))
				{
					mDrawGrid = !mDrawGrid;
				}
				if (preDrawGrid)
					ImGui::PopStyleColor();
			}

		}
	}

	void SceneWindow::Update(float dt)
	{
		if (D_CAMERA_MANAGER::SetViewportDimansion(mWidth, mHeight))
		{
			CreateBuffers();
		}

		if (mOrbitCam.IsAdjusting() || (D_KEYBOARD::GetKey(D_KEYBOARD::Keys::LeftAlt) && D_MOUSE::GetButton(D_MOUSE::Keys::Left) && mHovered))
		{
			mOrbitCam.Update(dt);
			mFlyingCam.SetOrientationDirty();
			mMovingCam = true;
		}
		else if (D_MOUSE::GetButton(D_MOUSE::Keys::Right) && mHovered)
		{
			mFlyingCam.Update(dt);
			mOrbitCam.SetTargetLocationDirty();
			mMovingCam = true;
		}
		else
		{
			mCamera.Update();
			mMovingCam = false;
		}

		if (mMovingCam)
			return;

		// Shortcuts
		if (!mFocused)
			return;

		// Focusing on object
		auto selectedGameObject = D_EDITOR_CONTEXT::GetSelectedGameObject();
		if (D_KEYBOARD::IsKeyDown(D_KEYBOARD::Keys::F) && selectedGameObject)
		{
			mOrbitCam.SetTarget(selectedGameObject->GetTransform().Translation);
		}

		if (D_KEYBOARD::IsKeyDown(D_KEYBOARD::Keys::W))
			mManipulateOperation = ImGuizmo::OPERATION::TRANSLATE;
		else if (D_KEYBOARD::IsKeyDown(D_KEYBOARD::Keys::E))
			mManipulateOperation = ImGuizmo::OPERATION::ROTATE;
		else if (D_KEYBOARD::IsKeyDown(D_KEYBOARD::Keys::R))
			mManipulateOperation = ImGuizmo::OPERATION::SCALE;
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
		const Mesh* mesh = mLineMeshResource->GetMeshData();
		item.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		item.IndexCount = mesh->mDraw[0].IndexCount;
		item.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		item.Mesh = mesh;
		item.PsoFlags = RenderItem::ColorOnly;
		item.PsoType = D_RENDERER::PipelineStateTypes::ColorPso;
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

	void SceneWindow::AddWindowRenderItems(D_RENDERER::MeshSorter& sorter)
	{
		for (auto& item : mWindowRenderItems)
		{
			sorter.AddMesh(item, 1);
		}
	}

	void SceneWindow::AddSceneRenderItems(D_RENDERER::MeshSorter& sorter)
	{
		auto& worldReg = D_WORLD::GetRegistry();

		auto cam = D_CAMERA_MANAGER::GetActiveCamera();
		auto frustum = cam->GetViewSpaceFrustum();

		// Iterating over meshes
		worldReg.each([&](D_ECS_COMP::MeshRendererComponent& meshComp)
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
		worldReg.each([&](D_ECS_COMP::SkeletalMeshRendererComponent& meshComp)
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

		//D_LOG_DEBUG("Number of render items: " + std::to_string(items.size()));
	}
}
