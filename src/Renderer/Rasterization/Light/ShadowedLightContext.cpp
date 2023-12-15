#include "Renderer/pch.hpp"
#include "ShadowedLightContext.hpp"

#include "Renderer/Rasterization/Renderer.hpp"
#include "Renderer/RendererCommon.hpp"

#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <Job/Job.hpp>
#include <Math/VectorMath.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>

#include "Renderer/Components/LightComponent.hpp"

using namespace D_MATH;
using namespace D_RENDERER_LIGHT;

namespace Darius::Renderer::Rasterization::Light
{
	RasterizationShadowedLightContext::RasterizationShadowedLightContext(UINT directionalShadowBufferDimansion, UINT pointShadowBufferDimansion, UINT spotShadowBufferDimansion, UINT shadowBufferDepthPercision) :
		LightContext(),
		mDirectionalShadowBufferWidth(directionalShadowBufferDimansion),
		mPointShadowBufferWidth(pointShadowBufferDimansion),
		mSpotShadowBufferWidth(spotShadowBufferDimansion),
		mShadowBufferDepthPercision(shadowBufferDepthPercision)
	{
		mShadowBuffersDirectional.resize(D_RENDERER_LIGHT::MaxNumDirectionalLight);
		mShadowBuffersPoint.resize(D_RENDERER_LIGHT::MaxNumPointLight * 6);
		mShadowBuffersSpot.resize(D_RENDERER_LIGHT::MaxNumSpotLight);

		for (int i = 0; i < mShadowBuffersDirectional.size(); i++)
			mShadowBuffersDirectional[i].Create(std::wstring(L"Directional Shadow Buffer ") + std::to_wstring(i), mDirectionalShadowBufferWidth, mDirectionalShadowBufferWidth, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
		for (int i = 0; i < mShadowBuffersPoint.size(); i++)
			mShadowBuffersPoint[i].Create(std::wstring(L"Point Shadow Buffer ") + std::to_wstring(i % 6) + L"/6 " + std::to_wstring(i / 6), mPointShadowBufferWidth, mPointShadowBufferWidth, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
		for (int i = 0; i < mShadowBuffersSpot.size(); i++)
			mShadowBuffersSpot[i].Create(std::wstring(L"Spot Shadow Buffer ") + std::to_wstring(i), mSpotShadowBufferWidth, mSpotShadowBufferWidth, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

		// Initializing main shadow buffers
		auto shadowBufferFormat = D_GRAPHICS::GetShadowFormat();
		mDirectionalShadowTextureArrayBuffer.CreateArray(L"Directional Shadow Texture Array Buffer", mDirectionalShadowBufferWidth, mDirectionalShadowBufferWidth, D_RENDERER_LIGHT::MaxNumDirectionalLight, DXGI_FORMAT_R16_UNORM);
		mPointShadowTextureArrayBuffer.CreateArray(L"Point Shadow Texture Array Buffer", mPointShadowBufferWidth, mPointShadowBufferWidth, 6 * D_RENDERER_LIGHT::MaxNumPointLight, DXGI_FORMAT_R16_UNORM, true);
		mSpotShadowTextureArrayBuffer.CreateArray(L"Spot Shadow Texture Array Buffer", mSpotShadowBufferWidth, mSpotShadowBufferWidth, D_RENDERER_LIGHT::MaxNumSpotLight, DXGI_FORMAT_R16_UNORM);

		// Create shadow cameras
		mDirectionalShadowCameras.resize(D_RENDERER_LIGHT::MaxNumDirectionalLight);
		mSpotShadowCameras.resize(D_RENDERER_LIGHT::MaxNumSpotLight);
		mPointShadowCameras.resize(D_RENDERER_LIGHT::MaxNumPointLight * 6);
	}

	RasterizationShadowedLightContext::~RasterizationShadowedLightContext()
	{
		mDirectionalShadowTextureArrayBuffer.Destroy();
		mPointShadowTextureArrayBuffer.Destroy();
		mSpotShadowTextureArrayBuffer.Destroy();

		for (int i = 0; i < mShadowBuffersDirectional.size(); i++)
			mShadowBuffersDirectional[i].Destroy();
		for (int i = 0; i < mShadowBuffersPoint.size(); i++)
			mShadowBuffersPoint[i].Destroy();
		for (int i = 0; i < mShadowBuffersSpot.size(); i++)
			mShadowBuffersSpot[i].Destroy();
	}

	void RasterizationShadowedLightContext::Update(D_MATH_CAMERA::Camera const& viewerCamera)
	{
		Reset();

		D_CONTAINERS::DVector<std::function<void()>> updateFuncs;
		updateFuncs.reserve(D_WORLD::CountComponents<D_RENDERER::LightComponent>());

		D_WORLD::IterateComponents<D_RENDERER::LightComponent>([&](D_RENDERER::LightComponent& comp)
			{
				if (!comp.IsActive())
					return;

				auto lightType = comp.GetLightType();
				UINT lightIndex;
				auto& lightData = AddLightSource(comp.GetLightData(), lightType, lightIndex);
				auto trans = comp.GetTransform();
				lightData.Position = (DirectX::XMFLOAT3)trans->GetPosition();
				lightData.Direction = (DirectX::XMFLOAT3)trans->GetRotation().GetForward();

				updateFuncs.push_back([&, lightType, viewerCamera, light = &lightData, lightIndex]()
					{
						switch (lightType)
						{
						case LightSourceType::DirectionalLight:
							CalculateDirectionalShadowCamera(viewerCamera, *light, lightIndex);
							break;
						case LightSourceType::PointLight:
							CalculatePointShadowCamera(*light, lightIndex);
							break;
						case LightSourceType::SpotLight:
							CalculateSpotShadowCamera(*light, lightIndex);
							break;
						default:
							D_ASSERT_M(false, "Source type is not implemented");
						}
					});
			});


		// Wait for processing all light sources
		D_JOB::AddTaskSetAndWait(updateFuncs);
	}

	void RasterizationShadowedLightContext::CalculateDirectionalShadowCamera(D_MATH_CAMERA::Camera const& viewerCamera, LightData& light, int directionalIndex)
	{
		auto& cam = mDirectionalShadowCameras[directionalIndex];
		auto lightDir = Vector3(light.Direction);
		cam.SetLookDirection(lightDir, Vector3::Up);
		cam.SetPosition(Vector3(kZero));
		cam.Update();
		auto& shadowView = cam.GetViewMatrix();

		auto const& viewrFrustom = viewerCamera.GetWorldSpaceFrustum();
		auto shadowSpaceAABB = D_MATH_BOUNDS::AxisAlignedBox();

		// Finding frustum aabb in shadow space
		for (int i = 0; i < D_MATH_CAMERA::Frustum::_kNumCorners; i++)
		{
			auto corner = viewrFrustom.GetFrustumCorner((D_MATH_CAMERA::Frustum::CornerID)i);
			auto shadowSpaceCorner = shadowView * corner;
			shadowSpaceAABB.AddPoint(Vector3(shadowSpaceCorner));
		}

		auto camWorldPos = D_MATH::Invert(shadowView) * shadowSpaceAABB.GetCenter();
		auto dim = shadowSpaceAABB.GetDimensions();

		// Fitting directional light bounds to the frustom... With extra Z
		cam.UpdateMatrix(lightDir, Vector3(camWorldPos), dim, mDirectionalShadowBufferWidth, mDirectionalShadowBufferWidth, mShadowBufferDepthPercision);

		light.ShadowMatrix = cam.GetShadowMatrix();
	}

	void RasterizationShadowedLightContext::RenderDirectionalShadow(D_RENDERER_RAST::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData const& light, int directionalIndex)
	{
		auto const& cam = mDirectionalShadowCameras[directionalIndex];

		GlobalConstants globals;
		globals.ViewProj = cam.GetViewProjMatrix();
		globals.CameraPos = (DirectX::XMFLOAT3)cam.GetPosition();
		globals.ShadowTexelSize.x = 1.f / mDirectionalShadowBufferWidth;
		auto const& frustum = cam.GetWorldSpaceFrustum();
		for (int i = 0; i < 6; i++)
			globals.FrustumPlanes[i] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)i);

		auto& shadowBuffer = mShadowBuffersDirectional[directionalIndex];
		shadowBuffer.BeginRendering(context);

		sorter.SetCamera(cam);
		sorter.SetDepthStencilTarget(shadowBuffer, nullptr);
		sorter.RenderMeshes(MeshSorter::kZPass, context, nullptr, globals);

		context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		context.CopySubresource(mDirectionalShadowTextureArrayBuffer, directionalIndex, shadowBuffer, 0);
	}

	void RasterizationShadowedLightContext::CalculateSpotShadowCamera(LightData& light, int spotLightIndex)
	{
		auto& shadowCamera = mSpotShadowCameras[spotLightIndex];
		shadowCamera.SetEyeAtUp(light.Position, Vector3(light.Position) + Vector3(light.Direction), Vector3::Up);
		shadowCamera.SetPerspectiveMatrix(D_MATH::ACos(light.SpotAngles.y) * 2, 1.0f, light.Range * .001f, light.Range * 1.0f);
		shadowCamera.Update();
		light.ShadowMatrix = shadowCamera.GetViewProjMatrix();
	}

	void RasterizationShadowedLightContext::RenderSpotShadow(D_RENDERER_RAST::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData const& light, int spotLightIndex)
	{
		auto const& shadowCamera = mSpotShadowCameras[spotLightIndex];

		GlobalConstants globals;
		globals.ViewProj = shadowCamera.GetViewProjMatrix();
		globals.CameraPos = (DirectX::XMFLOAT3)shadowCamera.GetPosition();
		auto const& frustum = shadowCamera.GetWorldSpaceFrustum();
		for (int i = 0; i < 6; i++)
			globals.FrustumPlanes[i] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)i);

		auto& shadowBuffer = mShadowBuffersSpot[spotLightIndex];

		shadowBuffer.BeginRendering(context);

		sorter.SetCamera(shadowCamera);
		sorter.SetDepthStencilTarget(shadowBuffer, nullptr);
		sorter.RenderMeshes(MeshSorter::kZPass, context, nullptr, globals);

		context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		context.CopySubresource(mSpotShadowTextureArrayBuffer, spotLightIndex, shadowBuffer, 0);
	}

	void RasterizationShadowedLightContext::CalculatePointShadowCamera(LightData& light, int pointLightIndex)
	{
		static const float nearDist = 0.05f;
		// Left
		{
			auto& shadowCamera = mPointShadowCameras[pointLightIndex * 6];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Left, Vector3::Up);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
			light.ShadowMatrix = shadowCamera.GetProjMatrix();
		}

		// Right
		{
			auto& shadowCamera = mPointShadowCameras[pointLightIndex * 6 + 1];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Right, Vector3::Up);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

		// Top
		{
			auto& shadowCamera = mPointShadowCameras[pointLightIndex * 6 + 2];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Up, Vector3::Forward);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

		// Bottom
		{
			auto& shadowCamera = mPointShadowCameras[pointLightIndex * 6 + 3];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Down, Vector3::Backward);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

		// Back
		{
			auto& shadowCamera = mPointShadowCameras[pointLightIndex * 6 + 4];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Backward, Vector3::Up);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

		// Front
		{
			auto& shadowCamera = mPointShadowCameras[pointLightIndex * 6 + 5];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Forward, Vector3::Up);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

	}

	void RasterizationShadowedLightContext::RenderPointShadow(D_RENDERER_RAST::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData const& light, int pointLightIndex)
	{
		GlobalConstants globals;
		{
			auto& refCam = mPointShadowCameras[pointLightIndex * 6];
			globals.CameraPos = (DirectX::XMFLOAT3)refCam.GetPosition();
			globals.Proj = refCam.GetProjMatrix();
		}

		for (int i = 0; i < 6; i++)
		{
			auto const& shadowCamera = mPointShadowCameras[pointLightIndex * 6 + i];

			globals.ViewProj = shadowCamera.GetViewProjMatrix();
			auto const& frustum = shadowCamera.GetWorldSpaceFrustum();
			for (int j = 0; j < 6; j++)
				globals.FrustumPlanes[j] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)j);

			auto& shadowBuffer = mShadowBuffersPoint[pointLightIndex * 6 + i];

			shadowBuffer.BeginRendering(context);
			sorter.Reset();
			sorter.SetCamera(shadowCamera);
			sorter.SetDepthStencilTarget(shadowBuffer, nullptr);
			sorter.RenderMeshes(MeshSorter::kZPass, context, nullptr, globals);
			context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

			context.CopySubresource(mPointShadowTextureArrayBuffer, pointLightIndex * 6 + i, shadowBuffer, 0);
		}
	}

	void RasterizationShadowedLightContext::RenderShadows(D_CONTAINERS::DVector<RenderItem> const& shadowRenderItems, D_GRAPHICS::GraphicsContext& shadowContext)
	{

		D_PROFILING::ScopedTimer _prof(L"Rendering Shadows", shadowContext);

		// TODO: Create sorter and input render item per light source
		MeshSorter sorter(MeshSorter::kShadows);
		for (auto const& ri : shadowRenderItems) sorter.AddMesh(ri, 0.1f);

		sorter.Sort();

		shadowContext.TransitionResource(mDirectionalShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
		shadowContext.TransitionResource(mSpotShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
		shadowContext.TransitionResource(mPointShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);

		// Rendering directional
		for (UINT i = 0u; i < GetNumberOfDirectionalLights(); i++)
		{

			sorter.Reset();

			auto const& light = GetDirectionalLightData(i);
			if (!light.CastsShadow)
				continue;

			RenderDirectionalShadow(sorter, shadowContext, light, i);
		}
		sorter.SetViewport(D3D12_VIEWPORT());
		sorter.SetScissor({ 1, 1, (long)mSpotShadowBufferWidth - 2l, (long)mSpotShadowBufferWidth - 2l });

		// Rendering spot
		for (UINT idx = 0u; idx < MaxNumSpotLight; idx++)
		{
			if (idx >= GetNumberOfSpotLights())
				break;

			sorter.Reset();
			auto const& light = GetSpotLightData(idx);
			if (!light.CastsShadow)
				continue;

			RenderSpotShadow(sorter, shadowContext, light, idx);
		}
		sorter.SetViewport(D3D12_VIEWPORT());
		sorter.SetScissor({ 0l, 0l, (long)mPointShadowBufferWidth, (long)mPointShadowBufferWidth });

		// Rendering point
		for (UINT idx = 0u; idx < MaxNumPointLight; idx++)
		{
			if (idx >= GetNumberOfPointLights())
				break;

			sorter.Reset();
			auto const& light = GetPointLightData(idx);
			if (!light.CastsShadow)
				continue;

			RenderPointShadow(sorter, shadowContext, light, idx);
		}

		shadowContext.TransitionResource(mDirectionalShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		shadowContext.TransitionResource(mSpotShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		shadowContext.TransitionResource(mPointShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
	}
}

