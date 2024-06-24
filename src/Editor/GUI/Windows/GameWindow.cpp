#include "Editor/pch.hpp"
#include "GameWindow.hpp"

#include "Editor/GUI/GuiRenderer.hpp"
#include "Editor/GUI/GuiManager.hpp"

#include <Core/Input.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/PostProcessing/PostProcessing.hpp>
#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>
#include <Renderer/Components/MeshRendererComponent.hpp>
#include <Utils/StackWalker.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_GRAPHICS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_RENDERER;
using namespace DirectX;

namespace Darius::Editor::Gui::Windows
{
	GameWindow::GameWindow(D_SERIALIZATION::Json& config) :
		Window(config),
		mCustomDepthApplied(false),
		mRenderBuffers(),
		mCapturingMouse(false)
	{
		mRenderBuffers.SceneNormals = ColorBuffer(D_MATH::Color(0.f, 0.f, 0.f, 1.f));
		mRenderBuffers.SSAOFullScreen = ColorBuffer(D_MATH::Color(1.f, 1.f, 1.f));
		CreateBuffers();
		mTextureHandle = D_GUI_RENDERER::AllocateUiTexture();

		// Window padding
		mPadding[0] = mPadding[1] = 0.f;
	}

	GameWindow::~GameWindow()
	{
		mRenderBuffers.Destroy();
	}

	void GameWindow::Render()
	{

		auto& context = GraphicsContext::Begin(L"Game Window Render");

		auto camera = D_CAMERA_MANAGER::GetActiveCamera();

		if(!camera.IsValid() || !camera->IsActive() || !UpdateGlobalConstants(mSceneGlobals))
		{
			// Clearing depth
			context.TransitionResource(mRenderBuffers.SceneDepth, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
			context.ClearDepth(mRenderBuffers.SceneDepth);

			// Clear scene color
			context.TransitionResource(mRenderBuffers.SceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
			context.ClearColor(mRenderBuffers.SceneTexture);

			context.Finish();
			return;
		}

		D_MATH_CAMERA::Camera const& c = camera->GetCameraMath();


		D_RENDERER::RenderItemContext riContext;
		riContext.IsEditor = false;
		riContext.SelectedGameObject = nullptr;
		riContext.Shadow = false;
		riContext.StencilOverride = 0;

		auto skyboxSpecular = camera->GetSkyboxSpecularTexture();
		auto skyboxDiffuse = camera->GetSkyboxDiffuseTexture();

		bool drawSkybox = true;

		SceneRenderContext rc = SceneRenderContext::MakeFromBuffers(mRenderBuffers, mCustomDepthApplied, context, c, mSceneGlobals, riContext, {});
		rc.RadianceIBL = drawSkybox ? skyboxSpecular : nullptr;
		rc.IrradianceIBL = drawSkybox ? skyboxDiffuse : nullptr;
		rc.DrawSkybox = drawSkybox;

		// Post Processing
		D_GRAPHICS_PP::PostProcessContextBuffers postBuffers = mRenderBuffers.GetPostProcessingBuffers(L"Game Window");

		D_RENDERER::Render(L"Scene Window", rc, [context = &context, buffers = &postBuffers]()
			{
				D_GRAPHICS_PP::Render(*buffers, (*context).GetComputeContext());
			});

		context.TransitionResource(mRenderBuffers.SceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		context.Finish();

		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptorsSimple(1, mTextureHandle, mRenderBuffers.SceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	}

	void GameWindow::DrawGUI()
	{
		auto cam = D_CAMERA_MANAGER::GetActiveCamera();
		if(!cam.IsValid() || !cam->IsActive())
		{
			ImGui::Text("No ACTIVE CAMERA IN SCENE!");
		}
		else
			ImGui::Image((ImTextureID)mTextureHandle.GetGpuPtr(), ImVec2(mWidth, mHeight));

	}

	void GameWindow::Update(float)
	{
		if(D_CAMERA_MANAGER::SetViewportDimansion(mWidth, mHeight))
		{
			CreateBuffers();

			if(mCapturingMouse)
			{
				auto windowRect = GetRect();
				D_INPUT::ClipCursorToRect(&windowRect);
			}
		}

		if(mFocused)
		{
			bool ctrl = (D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyLControl) || D_INPUT::IsPressed(D_INPUT::DigitalInput::KeyRControl));
			bool f1 = D_INPUT::IsFirstPressed(D_INPUT::DigitalInput::KeyF1);

			static StackWalker sw;

			if(ctrl)
				D_LOG_DEBUG("Ctrl down");
			if(f1)
			{
				D_LOG_DEBUG("F1 just down");
				sw.ShowCallstack();
			}

			bool switchCapture = ctrl && f1;

			if(switchCapture)
			{
				if(mCapturingMouse)
				{
					mCapturingMouse = false;
					D_INPUT::SetCursorVisible(true);
					D_INPUT::ClipCursorToRect(nullptr);
				}
				else
				{
					mCapturingMouse = true;
					auto appRelativeRect = GetRect();
					RECT scrRect;
					D_GUI_MANAGER::MapWindowRectToScreen(appRelativeRect, scrRect);
					D_INPUT::SetCursorVisible(false);
					scrRect.left += 5;
					scrRect.top += 5;
					scrRect.bottom -= 5;
					scrRect.right -= 5;
					auto result = D_INPUT::ClipCursorToRect(&scrRect);
					(!result);
					(result);
				}
			}
		}
	}

	void GameWindow::CreateBuffers()
	{
		mCustomDepthApplied = D_GRAPHICS::IsCustomDepthEnable();

		D_GRAPHICS::GetCommandManager()->IdleGPU();
		mBufferWidth = mWidth;
		mBufferHeight = mHeight;

		mRenderBuffers.Create((UINT)mBufferWidth, (UINT)mBufferHeight, L"Game Window", mCustomDepthApplied);
	}

	bool GameWindow::UpdateGlobalConstants(GlobalConstants& globals)
	{
		Matrix4 temp;

		float width, height;
		D_CAMERA_MANAGER::GetViewportDimansion(width, height);

		auto time = *D_TIME::GetStepTimer();

		auto cameraP = D_CAMERA_MANAGER::GetActiveCamera();
		if(!cameraP.IsValid() || !cameraP->IsActive())
			return false;

		D_MATH_CAMERA::Camera const& camera = cameraP->GetCameraMath();

		temp = camera.GetViewMatrix(); // View
		globals.View = temp;
		globals.InvView = Matrix4::Inverse(temp);

		temp = camera.GetProjMatrix(); // Proj
		globals.Proj = temp;
		globals.InvProj = Matrix4::Inverse(temp);
		auto viewProjEyeCenter = temp * Matrix4::MakeLookToward(Vector3::Zero, camera.GetForwardVec(), camera.GetUpVec());
		globals.InvViewProjEyeCenter = Matrix4::Transpose(Matrix4::Inverse(viewProjEyeCenter));

		temp = camera.GetViewProjMatrix(); // View proj
		globals.ViewProj = temp;
		globals.InvViewProj = Matrix4::Inverse(temp);

		auto const pos = camera.GetPosition();
		globals.CameraPos = pos;
		globals.RenderTargetSize = XMFLOAT2(width, height);
		globals.InvRenderTargetSize = XMFLOAT2(1.f / width, 1.f / height);
		globals.NearZ = camera.GetNearClip();
		globals.FarZ = camera.GetFarClip();
		globals.TotalTime = (float)time.GetTotalSeconds();
		globals.DeltaTime = (float)time.GetElapsedSeconds();
		globals.AmbientLight = {0.1f, 0.1f, 0.1f, 1.0f};

		// TODO: Skybox For Game Window
		globals.IBLBias = 0.f;
		globals.IBLRange = 0.f;

		auto const& frustum = camera.GetWorldSpaceFrustum();

		for(int i = 0; i < 6; i++)
		{
			globals.FrustumPlanes[i] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)i);
		}

		return true;
	}

}
