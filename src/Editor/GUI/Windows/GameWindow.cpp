#include "Editor/pch.hpp"
#include "GameWindow.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Renderer/Components/MeshRendererComponent.hpp>
#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>
#include <Renderer/GraphicsCore.hpp>
#include <Renderer/GraphicsDeviceManager.hpp>
#include <Renderer/Light/LightManager.hpp>
#include <Renderer/PostProcessing/PostProcessing.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_GRAPHICS;
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
		mTemporalColor[0].Destroy();
		mTemporalColor[1].Destroy();
		mVelocityBuffer.Destroy();
		mLinearDepth[0].Destroy();
		mLinearDepth[1].Destroy();
		mExposureBuffer.Destroy();
		mLumaBuffer.Destroy();
		mHistogramBuffer.Destroy();
		mPostEffectsBuffer.Destroy();
	}

	void GameWindow::Render()
	{

		auto& context = GraphicsContext::Begin(L"Game Window Render");
		
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

		D_RENDERER::SceneRenderContext rc =
		{
			mSceneDepth,
			mSceneTexture,
			mVelocityBuffer,
			mTemporalColor,
			mLinearDepth,
			context,
			c,
			mSceneGlobals,
			true
		};

		D_RENDERER::Render(L"Scene Window", rc, nullptr,
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


		context.Finish();

		// Post Processing
		D_GRAPHICS_PP::PostProcessContextBuffers postBuffers =
		{
			mExposureBuffer,
			mSceneTexture,
			mLumaBuffer,
			mHistogramBuffer,
			mPostEffectsBuffer,
			L"Game Window"
		};
		D_GRAPHICS_PP::Render(postBuffers);

		// Copying to texture
		auto& copyContext = CommandContext::Begin(L"Game Window Texture Copy");
		copyContext.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		copyContext.Finish();

		D_GRAPHICS_DEVICE::GetDevice()->CopyDescriptorsSimple(1, mTextureHandle, mSceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

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
		mSceneTexture.Create(L"Game Scene Texture", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, D_GRAPHICS::GetColorFormat());
		mSceneDepth.Create(L"Game Scene DepthStencil", (UINT)mBufferWidth, (UINT)mBufferHeight, D_GRAPHICS::GetDepthFormat());

		// Linear Depth
		mLinearDepth[0].Create(L"Game Linear Depth 0", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16_UNORM);
		mLinearDepth[1].Create(L"Game Linear Depth 1", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16_UNORM);

		// Temporal Color 
		mTemporalColor[0].Create(L"Game Temporal Color 0", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
		mTemporalColor[1].Create(L"Game Temporal Color 1", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

		// Velocity Buffer
		mVelocityBuffer.Create(L"Game Motion Vectors", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R32_UINT);

		// Post Processing Buffers
		float exposure = D_GRAPHICS_PP::GetExposure();
		ALIGN_DECL_16 float initExposure[] =
		{
			exposure, 1.0f / exposure, exposure, 0.0f,
			D_GRAPHICS_PP::InitialMinLog, D_GRAPHICS_PP::InitialMaxLog, D_GRAPHICS_PP::InitialMaxLog - D_GRAPHICS_PP::InitialMinLog, 1.0f / (D_GRAPHICS_PP::InitialMaxLog - D_GRAPHICS_PP::InitialMinLog)
		};
		mExposureBuffer.Create(L"Scene Exposure", 8, 4, initExposure);
		mLumaBuffer.Create(L"Scene Luminance", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R8_UNORM);
		mHistogramBuffer.Create(L"Scene Histogram", 256, 4);
		mPostEffectsBuffer.Create(L"Scene Post Effects Buffer", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R32_UINT);

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

}
