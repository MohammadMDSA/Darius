#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "SceneWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Core/Input.hpp>
#include <Debug/DebugDraw.hpp>
#include <Engine/EngineContext.hpp>
#include <Scene/Scene.hpp>
#include <Renderer/Components/MeshRendererComponent.hpp>
#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>
#include <Renderer/PostProcessing/PostProcessing.hpp>
#include "Renderer/GraphicsCore.hpp"
#include "Renderer/GraphicsDeviceManager.hpp"
#include <Renderer/Renderer.hpp>
#include <Renderer/Light/LightManager.hpp>
#include <ResourceManager/ResourceManager.hpp>

#include <imgui.h>
#include <ImGuizmo.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

using namespace D_CONTAINERS;
using namespace D_GRAPHICS;
using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_RENDERER;
using namespace D_RENDERER_FRAME_RESOURCE;
using namespace D_RENDERER_GEOMETRY;
using namespace DirectX;

namespace Darius::Editor::Gui::Windows
{
	SceneWindow::SceneWindow(D_SERIALIZATION::Json const& config) :
		Window(config),
		mFlyingCam(mCamera, Vector3::Up),
		mOrbitCam(mCamera, D_MATH_BOUNDS::BoundingSphere(0.f, 0.f, 0.f, 5.f), Vector3::Up),
		mManipulateOperation(ImGuizmo::OPERATION::TRANSLATE),
		mManipulateMode(ImGuizmo::MODE::LOCAL),
		mDrawGrid(true),
		mDrawDebug(true),
		mDrawSkybox(true)
	{
		CreateBuffers();
		mTextureHandle = D_RENDERER::AllocateUiTexture(1);

		// Setup camera
		mCamera.SetFOV(XM_PI / 3);
		mCamera.SetZRange(0.001f, 10000.f);
		mCamera.SetPosition(Vector3(2.f, 2.f, 2.f));
		mCamera.SetLookDirection(Vector3(-2), Vector3::Up);
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

		auto diffIBLHandle = D_RESOURCE_LOADER::LoadResource(D_ENGINE_CONTEXT::GetAssetsPath() / "PBR/DefaultSkyboxDiffuseIBL_HDR.dds");
		auto specIBLHandle = D_RESOURCE_LOADER::LoadResource(D_ENGINE_CONTEXT::GetAssetsPath() / "PBR/DefaultSkyboxSpecularIBL.dds");

		//auto diffTex = D_RESOURCE::GetResource<TextureResource>(diffIBLHandle[0], this, L"Scene Window", "Editor Window");
		auto diffTex = D_RESOURCE::GetResource<TextureResource>(D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::TextureCubeMapBlack), this, L"Scene Window", "Editor Window");
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
		mTemporalColor[0].Destroy();
		mTemporalColor[1].Destroy();
		mVelocityBuffer.Destroy();
		mLinearDepth[0].Destroy();
		mLinearDepth[1].Destroy();
		mExposureBuffer.Destroy();
		mLumaBuffer.Destroy();
		mLumaLR.Destroy();
		mHistogramBuffer.Destroy();
		mPostEffectsBuffer.Destroy();
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

	void SceneWindow::Render()
	{

		UpdateGlobalConstants(mSceneGlobals);

		auto& context = GraphicsContext::Begin(L"Draw Scene Window");

		D_RENDERER::SceneRenderContext rc =
		{
			mSceneDepth,
			mSceneTexture,
			mVelocityBuffer,
			mTemporalColor,
			mLinearDepth,
			context,
			mCamera,
			mSceneGlobals,
			mDrawSkybox
		};

		D_RENDERER::Render(L"Scene Window", rc,
			[&](D_RENDERER::MeshSorter& sorter)
			{
				// Draw grid
				if (mDrawGrid)
					AddWindowRenderItems(sorter);
			},
			[&](D_RENDERER::MeshSorter& sorter)
			{
				// Add debug draw items
				if (mDrawDebug)
				{
					D_DEBUG_DRAW::GetRenderItems(sorter);
					sorter.Sort();
					sorter.RenderMeshes(MeshSorter::kTransparent, context, mSceneGlobals);
				}
			});

		context.Finish();

		// Post Processing
		D_GRAPHICS_PP::PostProcessContextBuffers postBuffers =
		{
			mExposureBuffer,
			mSceneTexture,
			mLumaBuffer,
			mLumaLR,
			mHistogramBuffer,
			mPostEffectsBuffer,
			L"Scene Window"
		};
		D_GRAPHICS_PP::Render(postBuffers);

		// Copying to texture
		auto& copyContext = CommandContext::Begin(L"Scene Window Texture Copy");
		copyContext.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		copyContext.Finish();

		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptorsSimple(1, mTextureHandle, mSceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
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
				min.y - 1.5f * padding.y + mHeight));

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
			mCamera.SetAspectRatio(mHeight / mWidth);
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
		mSceneTexture.Create(L"Scene Texture", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, D_GRAPHICS::GetColorFormat());
		mSceneDepth.Create(L"Scene DepthStencil", (UINT)mBufferWidth, (UINT)mBufferHeight, D_GRAPHICS::GetDepthFormat());

		// Linear Depth
		mLinearDepth[0].Create(L"Scene Linear Depth 0", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16_UNORM);
		mLinearDepth[1].Create(L"Scene Linear Depth 1", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16_UNORM);

		// Temporal Color 
		mTemporalColor[0].Create(L"Scene Temporal Color 0", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
		mTemporalColor[1].Create(L"Scene Temporal Color 1", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

		// Velocity Buffer
		mVelocityBuffer.Create(L"Scene Motion Vectors", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R32_UINT);

		// Divisible by 128 so that after dividing by 16, we still have multiples of 8x8 tiles.  The bloom
			// dimensions must be at least 1/4 native resolution to avoid undersampling.
			//uint32_t kBloomWidth = bufferWidth > 2560 ? Math::AlignUp(bufferWidth / 4, 128) : 640;
			//uint32_t kBloomHeight = bufferHeight > 1440 ? Math::AlignUp(bufferHeight / 4, 128) : 384;
		uint32_t bloomWidth = (UINT)mBufferWidth > 2560 ? 1280 : 640;
		uint32_t bloomHeight = (UINT)mBufferHeight > 1440 ? 768 : 384;

		// Post Processing Buffers
		float exposure = D_GRAPHICS_PP::GetExposure();
		ALIGN_DECL_16 float initExposure[] =
		{
			exposure, 1.0f / exposure, exposure, 0.0f,
			D_GRAPHICS_PP::InitialMinLog, D_GRAPHICS_PP::InitialMaxLog, D_GRAPHICS_PP::InitialMaxLog - D_GRAPHICS_PP::InitialMinLog, 1.0f / (D_GRAPHICS_PP::InitialMaxLog - D_GRAPHICS_PP::InitialMinLog)
		};
		mExposureBuffer.Create(L"Scene Exposure", 8, 4, initExposure);
		mLumaLR.Create(L"Scene Luma Buffer", bloomWidth, bloomHeight, 1, DXGI_FORMAT_R8_UINT);
		mLumaBuffer.Create(L"Scene Luminance", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R8_UNORM);
		mHistogramBuffer.Create(L"Scene Histogram", 256, 4);
		mPostEffectsBuffer.Create(L"Scene Post Effects Buffer", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R32_UINT);

	}

	void SceneWindow::CalcGridLineConstants(DVector<MeshConstants>& constants, int count)
	{
		auto scale = Matrix4::MakeScale((float)count * 2);
		auto rot = Matrix4::MakeLookAt(Vector3(kZero), Vector3(-1.f, 0.f, 0.f), Vector3::Up);

		for (short i = 0; i <= count; i++)
		{
			// Along +x
			constants.push_back({ Matrix4::MakeTranslation((float)i, 0.f, (float)count) * scale });

			// Along +z
			constants.push_back({ Matrix4::MakeTranslation(-(float)count, 0.f, (float)i) * rot * scale });

			if (i == 0)
				continue;

			// Along -x
			constants.push_back({ Matrix4::MakeTranslation(-(float)i, 0.f, (float)count) * scale });

			// Along -z
			constants.push_back({ Matrix4::MakeTranslation(-(float)count, 0.f, -(float)i) * rot * scale });
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

}
