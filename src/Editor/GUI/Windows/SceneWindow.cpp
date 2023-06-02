#include "Editor/pch.hpp"
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
		mDrawSkybox(true),
		mSceneNormals(D_MATH::Color(0.f, 0.f, 0.f, 1.f)),
		mSSAOFullScreen(D_MATH::Color(1.f, 1.f, 1.f)),
		mLineMeshResource({ L"Scene Window", rttr::type::get<SceneWindow>(), nullptr })
	{
		CreateBuffers();
		mTextureHandle = D_RENDERER::AllocateUiTexture(1);

		mLineMeshResource = D_RESOURCE::ResourceRef<D_GRAPHICS::BatchResource>({ L"Scene Window", rttr::type::get<SceneWindow>(), this });

		// Setup camera
		mCamera.SetFoV(XM_PI / 3);
		mCamera.SetZRange(0.001f, 10000.f);
		mCamera.SetPosition(Vector3(2.f, 2.f, 2.f));
		mCamera.SetLookDirection(Vector3(-2), Vector3::Up);
		mCamera.SetOrthographicSize(10);
		mCamera.SetOrthographic(false);
		ImGuizmo::SetOrthographic(true);

		mMouseWheelPerspectiveSensitivity = 0.1f;

		// Fetch line mesh resource
		auto lineHandle = D_GRAPHICS::GetDefaultGraphicsResource(D_GRAPHICS::DefaultResource::LineMesh);
		mLineMeshResource = D_RESOURCE::GetResource<BatchResource>(lineHandle);

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

		mSkyboxDiff = D_RESOURCE::GetResource<TextureResource>(diffIBLHandle[0], this, L"Scene Window", rttr::type::get<SceneWindow>());
		mSkyboxSpec = D_RESOURCE::GetResource<TextureResource>(specIBLHandle[0], this, L"Scene Window", rttr::type::get<SceneWindow>());

		D_RENDERER::SetIBLTextures(
			mSkyboxDiff,
			mSkyboxSpec
		);

		D_RENDERER::SetIBLBias(0);


		// Styling the gizmo
		ImGuizmo::AllowAxisFlip(false);
	}

	SceneWindow::~SceneWindow()
	{
		mSceneDepth.Destroy();
		mSceneTexture.Destroy();
		mSceneNormals.Destroy();
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
		mSSAOFullScreen.Destroy();
		mDepthDownsize1.Destroy();
		mDepthDownsize2.Destroy();
		mDepthDownsize3.Destroy();
		mDepthDownsize4.Destroy();
		mDepthTiled1.Destroy();
		mDepthTiled2.Destroy();
		mDepthTiled3.Destroy();
		mDepthTiled4.Destroy();
		mAOMerged1.Destroy();
		mAOMerged2.Destroy();
		mAOMerged3.Destroy();
		mAOMerged4.Destroy();
		mAOSmooth1.Destroy();
		mAOSmooth2.Destroy();
		mAOSmooth3.Destroy();
		mAOHighQuality1.Destroy();
		mAOHighQuality2.Destroy();
		mAOHighQuality3.Destroy();
		mAOHighQuality4.Destroy();
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

		auto const pos = mCamera.GetPosition();
		globals.CameraPos = pos;
		globals.RenderTargetSize = XMFLOAT2(width, height);
		globals.InvRenderTargetSize = XMFLOAT2(1.f / width, 1.f / height);
		globals.NearZ = mCamera.GetNearClip();
		globals.FarZ = mCamera.GetFarClip();
		globals.TotalTime = (float)time.GetTotalSeconds();
		globals.DeltaTime = (float)time.GetElapsedSeconds();
		globals.AmbientLight = { 0.1f, 0.1f, 0.1f, 0.1f };

	}

	void SceneWindow::Render()
	{

		UpdateGlobalConstants(mSceneGlobals);

		auto& context = GraphicsContext::Begin(L"Draw Scene Window");

		D_RENDERER::SceneRenderContext rc =
		{
			mSceneDepth,
			mSceneTexture,
			mSceneNormals,
			mVelocityBuffer,
			mTemporalColor,
			mLinearDepth,
			mSSAOFullScreen,
			mDepthDownsize1,
			mDepthDownsize2,
			mDepthDownsize3,
			mDepthDownsize4,
			mDepthTiled1,
			mDepthTiled2,
			mDepthTiled3,
			mDepthTiled4,
			mAOMerged1,
			mAOMerged2,
			mAOMerged3,
			mAOMerged4,
			mAOSmooth1,
			mAOSmooth2,
			mAOSmooth3,
			mAOHighQuality1,
			mAOHighQuality2,
			mAOHighQuality3,
			mAOHighQuality4,
			context,
			mCamera,
			mSceneGlobals,
			mDrawSkybox
		};

		MeshSorter sorter(MeshSorter::kDefault);
		D_RENDERER::Render(L"Scene Window", sorter, rc);

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
		D_GRAPHICS_PP::Render(postBuffers, context.GetComputeContext());

		// Drawing extra elements
		MeshSorter additionalSorter(sorter);

		// Draw grid
		if (mDrawGrid)
			AddWindowRenderItems(additionalSorter);

		// Add debug draw items
		if (mDrawDebug)
		{
			D_DEBUG_DRAW::GetRenderItems(additionalSorter);
			additionalSorter.Sort();
			additionalSorter.RenderMeshes(MeshSorter::kTransparent, context, nullptr, mSceneGlobals);
		}

		// Copying to texture
		context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(mSceneNormals, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

		context.Finish();

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
					if (!mDrawSkybox)
					{
						auto invTex = D_RESOURCE::ResourceRef<TextureResource>();
						D_RENDERER::SetIBLTextures(invTex, invTex);
					}
					else
					{
						D_RENDERER::SetIBLTextures(mSkyboxDiff, mSkyboxSpec);
					}
				}
				if (preDrawSkybox)
					ImGui::PopStyleColor();
			}

			ImGui::SameLine();
			// Options drop down
			{
				if (ImGui::BeginCombo("##SceneWindowOptionsDropDown", "Options", ImGuiComboFlags_HeightLarge))
				{
					// Wireframe
					{
						static char const* const selectedValue = ICON_FA_SQUARE_CHECK " Wireframe";
						static char const* const unselectedValue = ICON_FA_SQUARE " Wireframe";
						if (ImGui::Selectable(mForceWireframe ? selectedValue : unselectedValue, false, 0))
						{
							mForceWireframe = !mForceWireframe;
							D_RENDERER::SetForceWireframe(mForceWireframe);
						}

					}


					ImGui::EndCombo();
				}
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
		D_GRAPHICS::GetCommandManager()->IdleGPU();
		mBufferWidth = mWidth;
		mBufferHeight = mHeight;
		mSceneTexture.Create(L"Scene Texture", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, D_GRAPHICS::GetColorFormat());
		mSceneDepth.Create(L"Scene DepthStencil", (UINT)mBufferWidth, (UINT)mBufferHeight, D_GRAPHICS::GetDepthFormat());
		mSceneNormals.Create(L"Scene Normals", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

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

		// Ambient Occlusion Buffers
		const uint32_t bufferWidth1 = ((UINT)mBufferWidth + 1) / 2;
		const uint32_t bufferWidth2 = ((UINT)mBufferWidth + 3) / 4;
		const uint32_t bufferWidth3 = ((UINT)mBufferWidth + 7) / 8;
		const uint32_t bufferWidth4 = ((UINT)mBufferWidth + 15) / 16;
		const uint32_t bufferWidth5 = ((UINT)mBufferWidth + 31) / 32;
		const uint32_t bufferWidth6 = ((UINT)mBufferWidth + 63) / 64;
		const uint32_t bufferHeight1 = ((UINT)mBufferHeight + 1) / 2;
		const uint32_t bufferHeight2 = ((UINT)mBufferHeight + 3) / 4;
		const uint32_t bufferHeight3 = ((UINT)mBufferHeight + 7) / 8;
		const uint32_t bufferHeight4 = ((UINT)mBufferHeight + 15) / 16;
		const uint32_t bufferHeight5 = ((UINT)mBufferHeight + 31) / 32;
		const uint32_t bufferHeight6 = ((UINT)mBufferHeight + 63) / 64;
		mSSAOFullScreen.Create(L"Scene SSAO Full Res", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R8_UNORM);
		mDepthDownsize1.Create(L"Scene Depth Down-Sized 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R32_FLOAT);
		mDepthDownsize2.Create(L"Scene Depth Down-Sized 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R32_FLOAT);
		mDepthDownsize3.Create(L"Scene Depth Down-Sized 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R32_FLOAT);
		mDepthDownsize4.Create(L"Scene Depth Down-Sized 4", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R32_FLOAT);
		mDepthTiled1.CreateArray(L"Scene Depth De-Interleaved 1", bufferWidth3, bufferHeight3, 16, DXGI_FORMAT_R16_FLOAT);
		mDepthTiled2.CreateArray(L"Scene Depth De-Interleaved 2", bufferWidth4, bufferHeight4, 16, DXGI_FORMAT_R16_FLOAT);
		mDepthTiled3.CreateArray(L"Scene Depth De-Interleaved 3", bufferWidth5, bufferHeight5, 16, DXGI_FORMAT_R16_FLOAT);
		mDepthTiled4.CreateArray(L"Scene Depth De-Interleaved 4", bufferWidth6, bufferHeight6, 16, DXGI_FORMAT_R16_FLOAT);
		mAOMerged1.Create(L"Scene AO Re-Interleaved 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM);
		mAOMerged2.Create(L"Scene AO Re-Interleaved 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM);
		mAOMerged3.Create(L"Scene AO Re-Interleaved 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM);
		mAOMerged4.Create(L"Scene AO Re-Interleaved 4", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R8_UNORM);
		mAOSmooth1.Create(L"Scene AO Smoothed 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM);
		mAOSmooth2.Create(L"Scene AO Smoothed 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM);
		mAOSmooth3.Create(L"Scene AO Smoothed 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM);
		mAOHighQuality1.Create(L"Scene AO High Quality 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM);
		mAOHighQuality2.Create(L"Scene AO High Quality 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM);
		mAOHighQuality3.Create(L"Scene AO High Quality 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM);
		mAOHighQuality4.Create(L"Scene AO High Quality 4", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R8_UNORM);
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
		item.PsoFlags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | RenderItem::ColorOnly | RenderItem::TwoSided | RenderItem::LineOnly | RenderItem::AlphaBlend;
		item.PsoType = D_RENDERER::GetPso({ item.PsoFlags });
		item.DepthPsoIndex = D_RENDERER::GetPso({ (UINT16)(item.PsoFlags | RenderItem::DepthOnly) });
		item.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		item.MeshVsCBV = mLineConstantsGPU.GetGpuVirtualAddress();

		for (int i = 0; i < count; i++)
		{
			if (i / 2 == 0)
				item.Color = { 1.f, 1.f, 1.f, 0.8f };
			else if (((i - 2) / 4) % 10 == 9)
				item.Color = { 0.501f, 0.501f, 0.501f, 0.8f };
			else
				item.Color = { 0.3f, 0.3f, 0.3f, 0.8f };

			items.push_back(item);
			item.MeshVsCBV += sizeof(MeshConstants);
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
