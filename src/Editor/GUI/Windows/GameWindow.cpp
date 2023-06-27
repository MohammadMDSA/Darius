#include "Editor/pch.hpp"
#include "GameWindow.hpp"

#include <Core/TimeManager/TimeManager.hpp>
#include <Debug/DebugDraw.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Graphics/GraphicsCore.hpp>
#include <Graphics/GraphicsDeviceManager.hpp>
#include <Graphics/PostProcessing/PostProcessing.hpp>
#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>
#include <Renderer/Components/MeshRendererComponent.hpp>
#include <Renderer/Rasterization/Light/LightManager.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

using namespace D_MATH;
using namespace D_MATH_BOUNDS;
using namespace D_GRAPHICS;
using namespace D_RENDERER_RAST;
using namespace D_RENDERER_FRAME_RESOURCE;
using namespace DirectX;

namespace Darius::Editor::Gui::Windows
{
	GameWindow::GameWindow(D_SERIALIZATION::Json const& config) :
		Window(config),
		mSceneNormals(D_MATH::Color(0.f, 0.f, 0.f, 1.f)),
		mSSAOFullScreen(D_MATH::Color(1.f, 1.f, 1.f))
	{
		CreateBuffers();
		mTextureHandle = D_RENDERER_RAST::AllocateUiTexture();

		// Window padding
		mPadding[0] = mPadding[1] = 0.f;

	}

	GameWindow::~GameWindow()
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
		BloomUAV1[0].Destroy();
		BloomUAV1[1].Destroy();
		BloomUAV2[0].Destroy();
		BloomUAV2[1].Destroy();
		BloomUAV3[0].Destroy();
		BloomUAV3[1].Destroy();
		BloomUAV4[0].Destroy();
		BloomUAV4[1].Destroy();
		BloomUAV5[0].Destroy();
		BloomUAV5[1].Destroy();
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
			
			context.Finish();
			return;
		}
		
		D_MATH_CAMERA::Camera const& c = camera.Get()->GetCamera();

		D_RENDERER_RAST::SceneRenderContext rc =
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
			c,
			mSceneGlobals,
			true
		};

		MeshSorter sorter(MeshSorter::kDefault);
		D_RENDERER_RAST::Render(L"Scene Window", sorter, rc);


		// Post Processing
		D_GRAPHICS_PP::PostProcessContextBuffers postBuffers =
		{
			mExposureBuffer,
			mSceneTexture,
			mLumaBuffer,
			mLumaLR,
			mHistogramBuffer,
			mPostEffectsBuffer,
			BloomUAV1,
			BloomUAV2,
			BloomUAV3,
			BloomUAV4,
			BloomUAV5,
			L"Game Window"
		};
		D_GRAPHICS_PP::Render(postBuffers, context.GetComputeContext());

		context.TransitionResource(mSceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		context.Finish();

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
		D_GRAPHICS::GetCommandManager()->IdleGPU();
		mBufferWidth = mWidth;
		mBufferHeight = mHeight;
		mSceneTexture.Create(L"Game Scene Texture", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, D_GRAPHICS::GetColorFormat());
		mSceneDepth.Create(L"Game Scene DepthStencil", (UINT)mBufferWidth, (UINT)mBufferHeight, D_GRAPHICS::GetDepthFormat());
		mSceneNormals.Create(L"Game Normals", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

		// Linear Depth
		mLinearDepth[0].Create(L"Game Linear Depth 0", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16_UNORM);
		mLinearDepth[1].Create(L"Game Linear Depth 1", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16_UNORM);

		// Temporal Color 
		mTemporalColor[0].Create(L"Game Temporal Color 0", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);
		mTemporalColor[1].Create(L"Game Temporal Color 1", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R16G16B16A16_FLOAT);

		// Velocity Buffer
		mVelocityBuffer.Create(L"Game Motion Vectors", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R32_UINT);

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
		mExposureBuffer.Create(L"Game Exposure", 8, 4, initExposure);
		mLumaLR.Create(L"Game Luma Buffer", bloomWidth, bloomHeight, 1, DXGI_FORMAT_R8_UINT);
		BloomUAV1[0].Create(L"Scene Bloom Buffer 1a", bloomWidth, bloomHeight, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV1[1].Create(L"Scene Bloom Buffer 1b", bloomWidth, bloomHeight, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV2[0].Create(L"Scene Bloom Buffer 2a", bloomWidth / 2, bloomHeight / 2, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV2[1].Create(L"Scene Bloom Buffer 2b", bloomWidth / 2, bloomHeight / 2, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV3[0].Create(L"Scene Bloom Buffer 3a", bloomWidth / 4, bloomHeight / 4, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV3[1].Create(L"Scene Bloom Buffer 3b", bloomWidth / 4, bloomHeight / 4, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV4[0].Create(L"Scene Bloom Buffer 4a", bloomWidth / 8, bloomHeight / 8, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV4[1].Create(L"Scene Bloom Buffer 4b", bloomWidth / 8, bloomHeight / 8, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV5[0].Create(L"Scene Bloom Buffer 5a", bloomWidth / 16, bloomHeight / 16, 1, D_GRAPHICS::GetColorFormat());
		BloomUAV5[1].Create(L"Scene Bloom Buffer 5b", bloomWidth / 16, bloomHeight / 16, 1, D_GRAPHICS::GetColorFormat());
		mLumaBuffer.Create(L"Game Luminance", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R8_UNORM);
		mHistogramBuffer.Create(L"Game Histogram", 256, 4);
		mPostEffectsBuffer.Create(L"Game Post Effects Buffer", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R32_UINT);

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
		mSSAOFullScreen.Create(L"Game SSAO Full Res", (UINT)mBufferWidth, (UINT)mBufferHeight, 1, DXGI_FORMAT_R8_UNORM);
		mDepthDownsize1.Create(L"Game Depth Down-Sized 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R32_FLOAT);
		mDepthDownsize2.Create(L"Game Depth Down-Sized 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R32_FLOAT);
		mDepthDownsize3.Create(L"Game Depth Down-Sized 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R32_FLOAT);
		mDepthDownsize4.Create(L"Game Depth Down-Sized 4", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R32_FLOAT);
		mDepthTiled1.CreateArray(L"Game Depth De-Interleaved 1", bufferWidth3, bufferHeight3, 16, DXGI_FORMAT_R16_FLOAT);
		mDepthTiled2.CreateArray(L"Game Depth De-Interleaved 2", bufferWidth4, bufferHeight4, 16, DXGI_FORMAT_R16_FLOAT);
		mDepthTiled3.CreateArray(L"Game Depth De-Interleaved 3", bufferWidth5, bufferHeight5, 16, DXGI_FORMAT_R16_FLOAT);
		mDepthTiled4.CreateArray(L"Game Depth De-Interleaved 4", bufferWidth6, bufferHeight6, 16, DXGI_FORMAT_R16_FLOAT);
		mAOMerged1.Create(L"Game AO Re-Interleaved 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM);
		mAOMerged2.Create(L"Game AO Re-Interleaved 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM);
		mAOMerged3.Create(L"Game AO Re-Interleaved 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM);
		mAOMerged4.Create(L"Game AO Re-Interleaved 4", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R8_UNORM);
		mAOSmooth1.Create(L"Game AO Smoothed 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM);
		mAOSmooth2.Create(L"Game AO Smoothed 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM);
		mAOSmooth3.Create(L"Game AO Smoothed 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM);
		mAOHighQuality1.Create(L"Game AO High Quality 1", bufferWidth1, bufferHeight1, 1, DXGI_FORMAT_R8_UNORM);
		mAOHighQuality2.Create(L"Game AO High Quality 2", bufferWidth2, bufferHeight2, 1, DXGI_FORMAT_R8_UNORM);
		mAOHighQuality3.Create(L"Game AO High Quality 3", bufferWidth3, bufferHeight3, 1, DXGI_FORMAT_R8_UNORM);
		mAOHighQuality4.Create(L"Game AO High Quality 4", bufferWidth4, bufferHeight4, 1, DXGI_FORMAT_R8_UNORM);

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

		auto const pos = camera.GetPosition();
		globals.CameraPos = pos;
		globals.RenderTargetSize = XMFLOAT2(width, height);
		globals.InvRenderTargetSize = XMFLOAT2(1.f / width, 1.f / height);
		globals.NearZ = camera.GetNearClip();
		globals.FarZ = camera.GetFarClip();
		globals.TotalTime = (float)time.GetTotalSeconds();
		globals.DeltaTime = (float)time.GetElapsedSeconds();
		globals.AmbientLight = { 0.1f, 0.1f, 0.1f, 1.0f };

		auto const& frustum = camera.GetWorldSpaceFrustum();

		for (int i = 0; i < 6; i++)
		{
			globals.FrustumPlanes[i] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)i);
		}

		return true;
	}

}
