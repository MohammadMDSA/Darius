#include "Editor/pch.hpp"
#include "SceneWindow.hpp"

#include "Editor/EditorContext.hpp"
#include "Editor/GUI/GuiRenderer.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Core/Input.hpp>
#include <Debug/DebugDraw.hpp>
#include <Engine/EngineContext.hpp>
#include <Graphics/PostProcessing/PostProcessing.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/GraphicsUtils/Buffers/ReadbackBuffer.hpp>
#include <Math/Bounds/BoundingPlane.hpp>
#include <Math/Ray.hpp>
#include <Renderer/Components/MeshRendererComponent.hpp>
#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>
#include <Renderer/RendererManager.hpp>
#include <Renderer/Rasterization/Renderer.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <Scene/Scene.hpp>

#include <imgui.h>
#include <ImGuizmo.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

using namespace D_CONTAINERS;
using namespace D_GRAPHICS;
using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_RENDERER;
using namespace D_RENDERER_GEOMETRY;
using namespace DirectX;

constexpr char const* CameraPositionKey = "CameraPositin";
constexpr char const* CameraRotationKey = "CameraRotation";

namespace Darius::Editor::Gui::Windows
{
	SceneWindow::SceneWindow(D_SERIALIZATION::Json& config) :
		Window(config),
		mFlyingCam(mCamera, Vector3::Up),
		mOrbitCam(mCamera, D_MATH_BOUNDS::BoundingSphere(0.f, 0.f, 0.f, 5.f), Vector3::Up),
		mManipulateOperation(ImGuizmo::OPERATION::TRANSLATE),
		mManipulateMode(ImGuizmo::MODE::LOCAL),
		mDrawGrid(true),
		mDrawDebug(true),
		mDrawSkybox(true),
		mDragSpawned(false),
		mForceWireframe(false),
		mLineMeshResource(),
		mGuiPostProcess(sSelectedGameObjectStencilValue)
	{

		mRenderBuffers.SceneNormals = D_GRAPHICS_BUFFERS::ColorBuffer(D_MATH::Color(0.f, 0.f, 0.f, 1.f));
		mRenderBuffers.SSAOFullScreen = D_GRAPHICS_BUFFERS::ColorBuffer(D_MATH::Color(1.f, 1.f, 1.f));

		CreateBuffers();
		mTextureHandle = D_GUI_RENDERER::AllocateUiTexture(1);

		// Loading config
		Vector3 camPos;
		if(!ReadVector3Config(CameraPositionKey, camPos))
		{
			camPos = Vector3(2.f, 2.f, 2.f);
		}

		Quaternion camRot;
		if(!ReadQuaternionConfig(CameraRotationKey, camRot))
		{
			camRot = Quaternion::FromForwardAndAngle(Vector3(-2));
		}

		// Setup camera
		mCamera.SetFoV(XM_PI / 3);
		mCamera.SetZRange(0.001f, 1000.f);
		mCamera.SetPosition(camPos);
		mCamera.SetRotation(camRot);
		mCamera.SetOrthographicSize(10);
		mCamera.SetOrthographic(false);
		ImGuizmo::SetOrthographic(true);

		mMouseWheelPerspectiveSensitivity = 0.1f;

		// Fetch line mesh resource
		auto lineHandle = D_RENDERER::GetDefaultGraphicsResource(D_RENDERER::DefaultResource::LineMesh);
		mLineMeshResource = D_RESOURCE::GetResourceSync<D_RENDERER::BatchResource>(lineHandle);

		// Initializing grid gpu constants
		auto count = 50;
		auto total = 2 * (2 * count + 1);
		DVector<MeshConstants> consts;
		CalcGridLineConstants(consts, count);
		mLineConstantsGPU.Create(L"Scene Window Grid GPU Buffer", total, sizeof(MeshConstants), consts.data());

		CreateGrid(mWindowRenderItems, total);

		// Window padding
		mPadding[0] = mPadding[1] = 0.f;

		D_RESOURCE_LOADER::LoadResourceAsync(D_ENGINE_CONTEXT::GetAssetsPath() / "PBR/DefaultSkyboxDiffuseIBL_HDR.dds", [&](auto const& resourceHandles)
			{
				auto diffIBLHandle = resourceHandles[0];
				mSkyboxDiff = D_RESOURCE::GetResourceSync<D_RENDERER::TextureResource>(diffIBLHandle, true);

				D_RESOURCE_LOADER::LoadResourceAsync(D_ENGINE_CONTEXT::GetAssetsPath() / "PBR/DaylightBox.dds", [&](auto const& resourceHandles2)
					{
						auto specIBLHandle = resourceHandles2[0];
						mSkyboxSpec = D_RESOURCE::GetResourceSync<D_RENDERER::TextureResource>(specIBLHandle, true);

						D_RENDERER::SetIBLTextures(
							mSkyboxDiff.Get(),
							mSkyboxSpec.Get()
						);
					});
			});

		D_RENDERER::SetIBLBias(0);


		// Styling the gizmo
		ImGuizmo::AllowAxisFlip(false);
	}

	SceneWindow::~SceneWindow()
	{
		mPostProcessedSceneTexture.Destroy();
		mPickerColor.Destroy();
		mPickerDepth.Destroy();

		mRenderBuffers.Destroy();

		mPickerReadback.Destroy();

		// Setting camera position to config
		WriteVector3Config(CameraPositionKey, mCamera.GetPosition());
		WriteQuaternionConfig(CameraRotationKey, mCamera.GetRotation());
	}

	void SceneWindow::UpdateGlobalConstants(GlobalConstants& globals) const
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
		auto viewProjEyeCenter = temp * Matrix4::MakeLookToward(Vector3::Zero, mCamera.GetForwardVec(), mCamera.GetUpVec());
		globals.InvViewProjEyeCenter = Matrix4::Transpose(Matrix4::Inverse(viewProjEyeCenter));

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
		globals.AmbientLight = {0.1f, 0.1f, 0.1f, 0.1f};
		globals.IBLBias = 0;
		if(mSkyboxDiff.IsValid() && mDrawSkybox)
			globals.IBLRange = std::max(0.f, (float)const_cast<ID3D12Resource*>(mSkyboxDiff->GetTextureData()->GetResource())->GetDesc().MipLevels - 1);
		else
			globals.IBLRange = 0;

		auto const& frustum = mCamera.GetWorldSpaceFrustum();

		for(int i = 0; i < 6; i++)
		{
			globals.FrustumPlanes[i] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)i);
		}

	}

	void SceneWindow::Render()
	{

		UpdateGlobalConstants(mSceneGlobals);

		auto& context = GraphicsContext::Begin(L"Draw Scene Window");

		DVector<DVector<RenderItem> const*> additional;

		// Draw grid
		if(mDrawGrid)
			additional.push_back(&mWindowRenderItems);

		// Add debug draw items
		if(mDrawDebug)
			additional.push_back(&D_DEBUG_DRAW::GetRenderItems());

		D_RENDERER::RenderItemContext riContext;
		riContext.IsEditor = true;
		riContext.SelectedGameObject = D_EDITOR_CONTEXT::GetSelectedGameObject();
		riContext.Shadow = false;
		riContext.StencilOverride = sSelectedGameObjectStencilValue;

		SceneRenderContext rc = SceneRenderContext::MakeFromBuffers(mRenderBuffers,
			mCustomDepthApplied,
			context,
			mCamera,
			mSceneGlobals,
			riContext,
			additional);

		rc.PickerDepthBuffer = &mPickerDepth;
		rc.PickerColorBuffer = &mPickerColor;
		rc.RadianceIBL = mDrawSkybox && mSkyboxSpec.IsValid() ? mSkyboxSpec.Get() : nullptr;
		rc.IrradianceIBL = mDrawSkybox && mSkyboxDiff.IsValid() ? mSkyboxDiff.Get() : nullptr;
		rc.DrawSkybox = (bool)mDrawSkybox;
		rc.RenderPickerData = true;


		// Post Processing
		D_GRAPHICS_PP::PostProcessContextBuffers postBuffers = mRenderBuffers.GetPostProcessingBuffers(L"Scene Window");


		D_RENDERER::Render(L"Scene Window", rc, [context = &context, &postBuffers]()
			{
				D_GRAPHICS_PP::Render(postBuffers, (*context).GetComputeContext());
			});


		// Gui PostProcessing
		mGuiPostProcess.Render(rc, mPostProcessedSceneTexture);

		// Copying to texture
		context.TransitionResource(mPostProcessedSceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		context.Finish();

		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptorsSimple(1, mTextureHandle, mPostProcessedSceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	}

	void SceneWindow::DrawGUI()
	{
		D_PROFILING::ScopedTimer profiling(L"Scene Window Draw GUI");
		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();
		auto windowContentMinX = mPosX + min.x;
		auto windowContentMinY = mPosY + min.y;
		ImGui::Image((ImTextureID)mTextureHandle.GetGpuPtr(), ImVec2(mWidth, mHeight));
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(windowContentMinX, windowContentMinY, mWidth, mHeight);
		auto view = mCamera.GetViewMatrix();
		auto proj = mCamera.GetProjMatrix();

		auto selectedObj = D_EDITOR_CONTEXT::GetSelectedGameObject();

		bool manipulating = false;
		if(selectedObj)
		{
			bool altDown = (D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyLAlt) || D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyRAlt));

			auto world = selectedObj->GetTransform()->GetWorld();
			if(ImGuizmo::Manipulate((float*)&view, (float*)&proj, (ImGuizmo::OPERATION)mManipulateOperation, (ImGuizmo::MODE)mManipulateMode, (float*)&world))
			{
				manipulating = true;

				// Drag spawn
				if(!mDragSpawned && !ImGui::GetIO().WantTextInput && (D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyLAlt) || D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyRAlt)))
				{
					auto dragSpawnedGo = D_WORLD::InstantiateGameObject(selectedObj, true);
					dragSpawnedGo->SetParent(selectedObj->GetParent(), D_SCENE::GameObject::AttachmentType::KeepWorld);
					D_EDITOR_CONTEXT::SetSelectedGameObject(dragSpawnedGo);
					mDragSpawned = true;
				}
				// Set world if no drag spawn
				else
					selectedObj->GetTransform()->SetWorld(world);
			}
			else
			{
				if(!altDown)
					mDragSpawned = false;
			}

		}

		// Picker
		if(!ImGuizmo::IsOver() && !D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyLAlt) && mHovered && D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::Mouse0))
		{
			SelectPickedGameObject();
		}


		// Drawing tool buttons
		ImGui::SetCursorPos({20.f, 30.f});
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
			for(size_t i = 0; i < 3; i++)
			{
				bool selected = (ImGuizmo::OPERATION)mManipulateOperation == operations[i];
				if(selected)
					ImGui::PushStyleColor(ImGuiCol_Button, {0.26f, 0.59f, 1.f, 1.f});
				if(ImGui::Button(OperationnNames[i].c_str()))
				{
					mManipulateOperation = operations[i];
				}
				if(selected)
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

			for(size_t i = 0; i < 2; i++)
			{
				bool selected = (ImGuizmo::MODE)mManipulateMode == modes[i];
				if(selected)
					ImGui::PushStyleColor(ImGuiCol_Button, {0.26f, 0.59f, 1.f, 1.f});
				if(ImGui::Button(modeNames[i].c_str()))
				{
					mManipulateMode = modes[i];
				}
				if(selected)
					ImGui::PopStyleColor();

				ImGui::SameLine();
			}

			ImGui::SameLine(230.f);

			// Grid settings
			{
				auto preDrawGrid = mDrawGrid;
				if(mDrawGrid)
					ImGui::PushStyleColor(ImGuiCol_Button, {0.26f, 0.59f, 1.f, 1.f});
				if(ImGui::Button(ICON_FA_TABLE_CELLS))
				{
					mDrawGrid = ~mDrawGrid;
				}
				if(preDrawGrid)
					ImGui::PopStyleColor();
			}

			ImGui::SameLine();
			// Skybox settings
			{
				auto preDrawSkybox = mDrawSkybox;
				if(mDrawSkybox)
					ImGui::PushStyleColor(ImGuiCol_Button, {0.26f, 0.59f, 1.f, 1.f});
				if(ImGui::Button(ICON_FA_MOUNTAIN_SUN))
				{
					mDrawSkybox = ~mDrawSkybox;
					if(!mDrawSkybox)
					{
						auto invTex = D_RESOURCE::ResourceRef<D_RENDERER::TextureResource>();
						D_RENDERER::SetIBLTextures(invTex.Get(), invTex.Get());
					}
					else
					{
						D_RENDERER::SetIBLTextures(mSkyboxDiff.Get(), mSkyboxSpec.Get());
					}
				}
				if(preDrawSkybox)
					ImGui::PopStyleColor();
			}

			ImGui::SameLine();
			// Options drop down
			{
				if(ImGui::BeginCombo("##SceneWindowOptionsDropDown", "Options", ImGuiComboFlags_HeightLarge))
				{
					// Wireframe
					{
						static char const* const selectedValue = ICON_FA_SQUARE_CHECK " Wireframe";
						static char const* const unselectedValue = ICON_FA_SQUARE " Wireframe";
						if(ImGui::Selectable(mForceWireframe ? selectedValue : unselectedValue, false, 0))
						{
							mForceWireframe = ~mForceWireframe;
							D_RENDERER::SetForceWireframe(mForceWireframe);
						}

					}


					ImGui::EndCombo();
				}
			}

			ImGui::SameLine();

			// Camera Settings
			{

				if(ImGui::Button(ICON_FA_VIDEO))
					ImGui::OpenPopup("SceneWindowCameraSettings");

				if(ImGui::BeginPopup("SceneWindowCameraSettings"))
				{
					// Near clip
					{
						float value = mCamera.GetNearClip();
						if(ImGui::DragFloat("Near Clip", &value, 0.1f, 0.01f, mCamera.GetFarClip() + 0.1f, "%.1f", ImGuiSliderFlags_AlwaysClamp))
						{
							mCamera.SetZRange(value, mCamera.GetFarClip());
						}
					}

					// Far clip
					{
						float value = mCamera.GetFarClip();
						if(ImGui::DragFloat("Far Clip", &value, 0.1f, mCamera.GetNearClip() + 0.1f, FLT_MAX, "%.1f", ImGuiSliderFlags_AlwaysClamp))
						{
							mCamera.SetZRange(mCamera.GetNearClip(), value);
						}
					}

					// Camera speed
					{
						float value = mFlyingCam.GetMoveSpeed();
						if(ImGui::DragFloat("Camera Speed", &value, 0.1f, 0.001f, 128.f, "%.3f", ImGuiSliderFlags_Logarithmic))
						{
							mFlyingCam.SetMoveSpeed(value);
						}
					}

					ImGui::EndPopup();
				}
			}

		}
		ImGui::SetCursorPos({20.f, 70.f});
		auto scrPos = ImGui::GetMousePos();
		mMouseSceneTexturePos = D_MATH::Vector2(scrPos.x - windowContentMinX, scrPos.y - windowContentMinY);

		// Drawing compass
		{
			auto padding = ImVec2(10.f, 20.f);
			auto compassSize = ImVec2(100.f, 100.f);
			ImGuizmo::ViewManipulate((float*)&view, mOrbitCam.GetCurrentCloseness(), ImVec2(mPosX + min.x + padding.x, mPosY + min.y + mHeight - compassSize.y - padding.y), compassSize, 0x10101010);

			auto projButtonSize = ImVec2(50.f, 20.f);

			ImGui::SetCursorPos(ImVec2(min.x + padding.x + (compassSize.x - projButtonSize.x) / 2,
				min.y - 1.5f * padding.y + mHeight));

			if(mCamera.IsOrthographic())
			{
				if(ImGui::Button(ICON_FA_BARS "ISO", projButtonSize))
				{
					mCamera.SetOrthographic(false);
				}
			}
			else
			{
				if(ImGui::Button(ICON_FA_CHEVRON_LEFT "Pers", projButtonSize))
				{
					mCamera.SetOrthographic(true);
				}
			}

			mCamera.SetTransform(OrthogonalTransform(view.Inverse()));
		}
	}

	void SceneWindow::Update(float dt)
	{
		if(mBufferHeight != mHeight || mBufferWidth != mWidth)
		{
			mCamera.SetAspectRatio(mHeight / mWidth);
			CreateBuffers();
		}


		if(mCamera.IsOrthographic())
		{
			float orthoZoom = D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseScroll) * mMouseWheelPerspectiveSensitivity;
			if(orthoZoom != 0.f)
				mCamera.SetOrthographicSize(mCamera.GetOrthographicSize() + orthoZoom);
		}

		if(mOrbitCam.IsAdjusting() || (D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyLAlt) && !mDragSpawned && D_INPUT::IsPressed(D_INPUT::DigitalInput::Mouse0) && mHovered))
		{
			mOrbitCam.Update(dt);
			mFlyingCam.SetOrientationDirty();
			mMovingCam = true;
		}
		else if(D_INPUT::IsPressed(D_INPUT::DigitalInput::Mouse1) && mHovered)
		{
			// Change camera speed with wheel
			{
				auto wheel = D_INPUT::GetAnalogInput(D_INPUT::AnalogInput::MouseScroll) * 50.f;
				float currentSpeed = mFlyingCam.GetMoveSpeed();
				currentSpeed += wheel / 10.f; // +5%
				mFlyingCam.SetMoveSpeed(D_MATH::Max(currentSpeed, 0.f));
			}


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

		if(mMovingCam)
			return;

		// Shortcuts
		if(ImGui::GetIO().WantTextInput)
			return;

		// Focusing on object
		auto selectedGameObject = D_EDITOR_CONTEXT::GetSelectedGameObject();
		if(D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeyF) && selectedGameObject)
		{
			mOrbitCam.SetTarget(selectedGameObject->GetTransform()->GetPosition());
		}

		if(D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeyW))
			mManipulateOperation = ImGuizmo::OPERATION::TRANSLATE;
		else if(D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeyE))
			mManipulateOperation = ImGuizmo::OPERATION::ROTATE;
		else if(D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeyR))
			mManipulateOperation = ImGuizmo::OPERATION::SCALE;
	}

	void SceneWindow::CreateBuffers()
	{

		mCustomDepthApplied = D_GRAPHICS::IsCustomDepthEnable();

		D_GRAPHICS::GetCommandManager()->IdleGPU();
		mBufferWidth = mWidth;
		mBufferHeight = mHeight;
		mPickerColor.Create(L"Editor Picker Color", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R32G32_UINT);
		mPostProcessedSceneTexture.Create(L"PostProcessed Scene Texture", (UINT)mBufferWidth, (UINT)mBufferHeight, 1u, D_GRAPHICS::GetColorFormat());
		mPickerDepth.Create(L"Editor Picker Depth", (UINT)mBufferWidth, (UINT)mBufferHeight, D_GRAPHICS::GetDepthFormat());
		mPickerReadback.Create(L"Editor Picker Readback", (UINT)(mBufferWidth * mBufferHeight), (UINT)D_GRAPHICS_BUFFERS::PixelBuffer::BytesPerPixel(DXGI_FORMAT_R32G32_UINT));

		mRenderBuffers.Create((UINT)mBufferWidth, (UINT)mBufferHeight, L"Scene Window", mCustomDepthApplied );
	}

	void SceneWindow::CalcGridLineConstants(DVector<MeshConstants>& constants, int count)
	{
		auto scale = Matrix4::MakeScale((float)count * 2);
		auto rot = Matrix4::MakeLookAt(Vector3(kZero), Vector3(-1.f, 0.f, 0.f), Vector3::Up);

		for(short i = 0; i <= count; i++)
		{
			// Along +x
			constants.push_back({Matrix4::MakeTranslation((float)i, 0.f, (float)count) * scale});

			// Along +z
			constants.push_back({Matrix4::MakeTranslation(-(float)count, 0.f, (float)i) * rot * scale});

			if(i == 0)
				continue;

			// Along -x
			constants.push_back({Matrix4::MakeTranslation(-(float)i, 0.f, (float)count) * scale});

			// Along -z
			constants.push_back({Matrix4::MakeTranslation(-(float)count, 0.f, -(float)i) * rot * scale});
		}
	}

	void SceneWindow::CreateGrid(DVector<RenderItem>& items, int count)
	{
		RenderItem item;
		const Mesh* mesh = mLineMeshResource->GetMeshData();
		item.BaseVertexLocation = mesh->mDraw[0].BaseVertexLocation;
		item.IndexCount = mesh->mDraw[0].IndexCount;
		item.StartIndexLocation = mesh->mDraw[0].StartIndexLocation;
		item.Mesh = mesh;
		item.PsoFlags = RenderItem::HasPosition | RenderItem::HasNormal | RenderItem::HasTangent | RenderItem::HasUV0 | RenderItem::ColorOnly | RenderItem::TwoSided | RenderItem::LineOnly | RenderItem::AlphaBlend;
		item.PsoType = D_RENDERER_RAST::GetPso({item.PsoFlags});
		item.DepthPsoIndex = D_RENDERER_RAST::GetPso({(UINT16)(item.PsoFlags | RenderItem::DepthOnly)});
		item.PrimitiveType = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		item.MeshVsCBV = mLineConstantsGPU.GetGpuVirtualAddress();

		for(int i = 0; i < count; i++)
		{
			if(i / 2 == 0)
				item.Color = {1.f, 1.f, 1.f, 0.8f};
			else if(((i - 2) / 4) % 10 == 9)
				item.Color = {0.501f, 0.501f, 0.501f, 0.8f};
			else
				item.Color = {0.3f, 0.3f, 0.3f, 0.8f};

			items.push_back(item);
			item.MeshVsCBV += sizeof(MeshConstants);
		}

	}

	D_MATH::Vector3 SceneWindow::SuggestSpawnPosition(float dist) const
	{
		return mCamera.GetForwardVec() * dist + mCamera.GetPosition();
	}

	D_MATH::Vector3 SceneWindow::SuggestSpawnPositionOnYPlane() const
	{
		D_MATH::Ray cameraRay = mCamera.GetCameraRay();
		D_MATH_BOUNDS::BoundingPlane yPlane(Vector3::Zero, Vector3::Up);

		float dist;
		if(cameraRay.Intersects(yPlane, dist))
		{
			return cameraRay.Resolve(dist);
		}

		Ray mirrorCameraRay(-cameraRay);
		if(mirrorCameraRay.Intersects(yPlane, dist))
		{
			return mirrorCameraRay.Resolve(dist);
		}

		return Vector3::Zero;
	}

	bool SceneWindow::SelectPickedGameObject()
	{
		auto& context = GraphicsContext::Begin(L"Picker Readback Fill");

		auto rowPitch = context.ReadbackTexture(mPickerReadback, mPickerColor);
		context.Finish(true);

		DirectX::XMUINT2 pixelPos =
		{
			D_MATH::Clamp((uint32_t)mMouseSceneTexturePos.GetX(), 0u, (uint32_t)mWidth),
			D_MATH::Clamp((uint32_t)mMouseSceneTexturePos.GetY(), 0u, (uint32_t)mHeight)
		};

		uint32_t pixelOffset = (pixelPos.y * (rowPitch / sizeof(DirectX::XMUINT2)) + pixelPos.x);

		auto pixelsData = mPickerReadback.Map();

		auto pixelData = ((DirectX::XMUINT2*)pixelsData)[pixelOffset];


		mPickerReadback.Unmap();

		uint64_t entityIdVal;
		std::memcpy(&entityIdVal, &pixelData, sizeof(uint64_t));

		D_ECS::Entity ent(entityIdVal);

		auto pickedGameObject = D_WORLD::GetGameObject(ent);

		D_EDITOR_CONTEXT::SetSelectedGameObject(pickedGameObject);

		return pickedGameObject != nullptr;
	}
}
