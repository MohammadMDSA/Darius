#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "SceneWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Core/Input.hpp>
#include <Debug/DebugDraw.hpp>
#include <Engine/EngineContext.hpp>
#include <Scene/Scene.hpp>
#include <Scene/EntityComponentSystem/Components/MeshRendererComponent.hpp>
#include <Scene/EntityComponentSystem/Components/SkeletalMeshRendererComponent.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/LightManager.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include <imgui.h>
#include <ImGuizmo.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

namespace Darius::Editor::Gui::Windows
{
	SceneWindow::SceneWindow(D_SERIALIZATION::Json const& config) :
		Window(config),
		mFlyingCam(mCamera, Vector3::Up()),
		mOrbitCam(mCamera, D_MATH_BOUNDS::BoundingSphere(0.f, 0.f, 0.f, 5.f), Vector3::Up()),
		mManipulateOperation(ImGuizmo::OPERATION::TRANSLATE),
		mManipulateMode(ImGuizmo::MODE::LOCAL),
		mDrawGrid(true),
		mDrawDebug(true),
		mDrawSkybox(true)
	{
		CreateBuffers();
		mTextureHandle = D_RENDERER::AllocateUiTexture(1);

		// Setup cameras
		mCamera.SetFOV(XM_PI / 3);
		mCamera.SetZRange(0.001f, 10000.f);
		mCamera.SetPosition(Vector3(2.f, 2.f, 2.f));
		mCamera.SetLookDirection(Vector3(-2), Vector3::Up());
		mCamera.SetOrthographicSize(10);
		mCamera.SetOrthographic(false);
		ImGuizmo::SetOrthographic(true);

		mMouseWheelPerspectiveSensitivity = 0.1f;


		// Fetch line mesh resource
		auto lineHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::LineMesh);
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

		auto diffIBLHandle = D_RESOURCE_LOADER::LoadResource(D_ENGINE_CONTEXT::GetAssetsPath() / "PBR/DefaultSkyboxDiffuseIBL.dds");
		auto specIBLHandle = D_RESOURCE_LOADER::LoadResource(D_ENGINE_CONTEXT::GetAssetsPath() / "PBR/DefaultSkyboxSpecularIBL.dds");

		auto diffTex = D_RESOURCE::GetResource<TextureResource>(diffIBLHandle[0], this, L"Scene Window", "Editor Window");
		auto specTex = D_RESOURCE::GetResource<TextureResource>(specIBLHandle[0], this, L"Scene Window", "Editor Window");

		D_RENDERER::SetIBLTextures(
			diffTex,
			specTex
		);

		D_RENDERER::SetIBLBias(0);


		// Styling the gizmo
		ImGuizmo::AllowAxisFlip(false);
	}

	SceneWindow::~SceneWindow()
	{
		mSceneDepth.Destroy();
		mSceneTexture.Destroy();
	}

	void SceneWindow::UpdateGlobalConstants(D_RENDERER_FRAME_RESOURCE::GlobalConstants& globals)
	{
		Matrix4 temp;

		float width = mWidth, height = mHeight;

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
		auto viewPort = CD3DX12_VIEWPORT(0.f, 0.f, mWidth, mHeight, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissor = CD3DX12_RECT(0l, 0l, (long)mWidth, (long)mHeight);
		MeshSorter sorter(MeshSorter::kDefault);
		sorter.SetCamera(mCamera);
		sorter.SetViewport(viewPort);
		sorter.SetScissor(scissor);
		sorter.SetDepthStencilTarget(mSceneDepth);
		sorter.AddRenderTarget(mSceneTexture);

		// Add meshes to sorter
		AddSceneRenderItems(sorter);

		{
			// Creating shadows

			DVector<RenderItem> shadowRenderItems;
			PopulateShadowRenderItems(shadowRenderItems);

			D_LIGHT::RenderShadows(shadowRenderItems);
		}

		// Draw grid
		if (mDrawGrid)
			AddWindowRenderItems(sorter);

		sorter.Sort();

		// Clearing scene color texture
		context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
		context.ClearColor(mSceneTexture);


		if (mDrawSkybox)
			D_RENDERER::DrawSkybox(context, mCamera, mSceneTexture, mSceneDepth, viewPort, scissor);

		sorter.RenderMeshes(MeshSorter::kTransparent, context, mSceneGlobals);

		// Add debug draw items
		if (mDrawDebug)
		{
			MeshSorter debugDrawSorter(sorter);
			D_DEBUG_DRAW::GetRenderItems(debugDrawSorter);
			debugDrawSorter.Sort();
			debugDrawSorter.RenderMeshes(MeshSorter::kTransparent, context, mSceneGlobals);
		}

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

			ImGui::SameLine();
			// Skybox settings
			{
				auto preDrawSkybox = mDrawSkybox;
				if (mDrawSkybox)
					ImGui::PushStyleColor(ImGuiCol_Button, { 0.26f, 0.59f, 1.f, 1.f });
				if (ImGui::Button(ICON_FA_MOUNTAIN_SUN))
				{
					mDrawSkybox = !mDrawSkybox;
				}
				if (preDrawSkybox)
					ImGui::PopStyleColor();
			}

		}

		// Drawing compass
		{
			auto padding = ImVec2(10.f, 20.f);
			auto compassSize = ImVec2(100.f, 100.f);
			ImGuizmo::ViewManipulate((float*)&view, mOrbitCam.GetCurrentCloseness(), ImVec2(mPosX + min.x + padding.x, mPosY + min.y + mHeight - compassSize.y - padding.y), compassSize, 0x10101010);

			auto projButtonSize = ImVec2(50.f, 20.f);

			ImGui::SetCursorPos(ImVec2(min.x + padding.x + (compassSize.x - projButtonSize.x) / 2,
				min.y - 1.5 * padding.y + mHeight));

			if (mCamera.IsOrthographic())
			{
				if (ImGui::Button(ICON_FA_BARS "ISO", projButtonSize))
				{
					mCamera.SetOrthographic(false);
				}
			}
			else
			{
				if (ImGui::Button(ICON_FA_CHEVRON_LEFT "Pers", projButtonSize))
				{
					mCamera.SetOrthographic(true);
				}
			}

			mCamera.SetTransform(OrthogonalTransform(view.Inverse()));
		}
	}

	void SceneWindow::Update(float dt)
	{
		if (mBufferHeight != mHeight || mBufferWidth != mWidth)
		{
			CreateBuffers();
		}


		if (mCamera.IsOrthographic())
		{
			float orthoZoom = (float)D_MOUSE::GetWheel() * mMouseWheelPerspectiveSensitivity;
			if (orthoZoom != 0.f)
				mCamera.SetOrthographicSize(mCamera.GetOrthographicSize() + orthoZoom);
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
			mFlyingCam.SetOrientationDirty();
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
		mBufferWidth = mWidth;
		mBufferHeight = mHeight;
		mSceneTexture.Create(L"Scene Texture", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, D_RENDERER_DEVICE::GetBackBufferFormat());
		mSceneDepth.Create(L"Scene DepthStencil", (UINT)mBufferWidth, (UINT)mBufferHeight, D_RENDERER_DEVICE::GetDepthBufferFormat());
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

	void SceneWindow::CreateGrid(DVector<D_RENDERER_FRAME_RESOURCE::RenderItem>& items, int count)
	{
		RenderItem item;
		const Mesh* mesh = mLineMeshResource->GetMeshData();
		item.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		item.IndexCount = mesh->mDraw[0].IndexCount;
		item.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		item.Mesh = mesh;
		item.PsoFlags = RenderItem::ColorOnly | RenderItem::TwoSided | RenderItem::LineOnly | RenderItem::AlphaBlend;
		item.PsoType = D_RENDERER::GetPso(RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | item.PsoFlags);
		item.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		item.MeshCBV = mLineConstantsGPU.GetGpuVirtualAddress();

		for (int i = 0; i < count; i++)
		{
			if (i / 2 == 0)
				item.Color = { 1.f, 1.f, 1.f, 0.8f };
			else if (((i - 2) / 4) % 10 == 9)
				item.Color = { 0.501f, 0.501f, 0.501f, 0.8f };
			else
				item.Color = { 0.3f, 0.3f, 0.3f, 0.8f };

			items.push_back(item);
			item.MeshCBV += sizeof(MeshConstants);
		}

	}

	void SceneWindow::AddWindowRenderItems(D_RENDERER::MeshSorter& sorter) const
	{
		for (auto& item : mWindowRenderItems)
		{
			sorter.AddMesh(item, 1);
		}
	}

	void SceneWindow::PopulateShadowRenderItems(D_CONTAINERS::DVector<RenderItem>& items) const
	{
		auto& worldReg = D_WORLD::GetRegistry();

		// Iterating over meshes
		worldReg.each([&](D_ECS_COMP::MeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				if (!meshComp.GetCastsShadow())
					return;

				items.push_back(meshComp.GetRenderItem());
			});

		// Iterating over meshes
		worldReg.each([&](D_ECS_COMP::SkeletalMeshRendererComponent& meshComp)
			{
				// Can't render
				if (!meshComp.CanRender())
					return;

				if (!meshComp.GetCastsShadow())
					return;

				items.push_back(meshComp.GetRenderItem());
			});
	}

	void SceneWindow::AddSceneRenderItems(D_RENDERER::MeshSorter& sorter) const
	{
		auto& worldReg = D_WORLD::GetRegistry();

		auto cam = &mCamera;
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

		//D_LOG_DEBUG("Number of render items: " << sorter.CountObjects());
	}
}
