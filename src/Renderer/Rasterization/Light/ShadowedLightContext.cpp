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
	RasterizationShadowedLightContext::RasterizationShadowedLightContext(bool createShadowBuffers, UINT directionalShadowBufferDimansion, UINT pointShadowBufferDimansion, UINT spotShadowBufferDimansion, UINT shadowBufferDepthPercision) :
		LightContext(),
		mDirectionalShadowBufferWidth(directionalShadowBufferDimansion),
		mPointShadowBufferWidth(pointShadowBufferDimansion),
		mSpotShadowBufferWidth(spotShadowBufferDimansion),
		mShadowBufferDepthPrecision(shadowBufferDepthPercision)
	{

		// Create buffer has been called in the parent constructor,
		// however, since virtual methods don't work in constructors,
		// we have to call CreateShadowBuffers separately
		if (createShadowBuffers)
			CreateShadowBuffers();


		mLightConfigBufferData.DirectionalShadowBufferTexelSize = 1.f / mDirectionalShadowBufferWidth;
		mLightConfigBufferData.SpotShadowBufferTexelSize = 1.f / mSpotShadowBufferWidth;
		mLightConfigBufferData.PointShadowBufferTexelSize = 1.f / mPointShadowBufferWidth;

	}

	RasterizationShadowedLightContext::~RasterizationShadowedLightContext()
	{
		DestroyBuffers();
	}

	void RasterizationShadowedLightContext::DestroyBuffers()
	{
		DestroyShadowBuffers();

		Super::DestroyBuffers();
	}

	void RasterizationShadowedLightContext::DestroyShadowBuffers()
	{
		D_GRAPHICS::GetCommandManager()->IdleGPU();

		mShadowDataUpload.Destroy();
		mShadowDataGpu.Destroy();

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

	void RasterizationShadowedLightContext::CreateBuffers()
	{
		Super::CreateBuffers();

		CreateShadowBuffers();
	}

	void RasterizationShadowedLightContext::CreateShadowBuffers()
	{
		mShadowBuffersDirectional.resize(D_RENDERER_LIGHT::MaxNumDirectionalLight * GetCascadesCount());
		mShadowBuffersPoint.resize(D_RENDERER_LIGHT::MaxNumPointLight * 6);
		mShadowBuffersSpot.resize(D_RENDERER_LIGHT::MaxNumSpotLight);

		for (int i = 0; i < mShadowBuffersDirectional.size(); i++)
			mShadowBuffersDirectional[i].Create(std::wstring(L"Directional Shadow Buffer ") + std::to_wstring(i), mDirectionalShadowBufferWidth, mDirectionalShadowBufferWidth, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
		for (int i = 0; i < mShadowBuffersPoint.size(); i++)
			mShadowBuffersPoint[i].Create(std::wstring(L"Point Shadow Buffer ") + std::to_wstring(i % 6) + L"/6 " + std::to_wstring(i / 6), mPointShadowBufferWidth, mPointShadowBufferWidth, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
		for (int i = 0; i < mShadowBuffersSpot.size(); i++)
			mShadowBuffersSpot[i].Create(std::wstring(L"Spot Shadow Buffer ") + std::to_wstring(i), mSpotShadowBufferWidth, mSpotShadowBufferWidth, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

		mLightConfigBufferData.CascadeMinBorderPadding = 1.f / mDirectionalShadowBufferWidth;
		mLightConfigBufferData.CascadeMaxBorderPadding = 1.f - mLightConfigBufferData.CascadeMinBorderPadding;

		// Initializing main shadow buffers
		auto shadowBufferFormat = D_GRAPHICS::GetShadowFormat();
		mDirectionalShadowTextureArrayBuffer.CreateArray(L"Directional Shadow Texture Array Buffer", mDirectionalShadowBufferWidth, mDirectionalShadowBufferWidth, D_RENDERER_LIGHT::MaxNumDirectionalLight * GetCascadesCount(), DXGI_FORMAT_R16_UNORM);
		mPointShadowTextureArrayBuffer.CreateArray(L"Point Shadow Texture Array Buffer", mPointShadowBufferWidth, mPointShadowBufferWidth, 6 * D_RENDERER_LIGHT::MaxNumPointLight, DXGI_FORMAT_R16_UNORM, true);
		mSpotShadowTextureArrayBuffer.CreateArray(L"Spot Shadow Texture Array Buffer", mSpotShadowBufferWidth, mSpotShadowBufferWidth, D_RENDERER_LIGHT::MaxNumSpotLight, DXGI_FORMAT_R16_UNORM);

		// Create shadow cameras
		mDirectionalShadowCameras.resize(D_RENDERER_LIGHT::MaxNumDirectionalLight * GetCascadesCount());
		mSpotShadowCameras.resize(D_RENDERER_LIGHT::MaxNumSpotLight);
		mPointShadowCameras.resize(D_RENDERER_LIGHT::MaxNumPointLight * 6);

		// Create shadow data buffers
		UINT shadowDataCount = MaxNumDirectionalLight * GetCascadesCount() + MaxNumPointLight + MaxNumSpotLight;
		mShadowData.resize(shadowDataCount);
		mShadowDataUpload.Create(L"Shadow Data Upload", shadowDataCount * sizeof(ShadowData), D_GRAPHICS_DEVICE::gNumFrameResources);
		mShadowDataGpu.Create(L"Shadow Data Gpu", shadowDataCount, sizeof(ShadowData));
	}

	void RasterizationShadowedLightContext::UpdateBuffers(D_GRAPHICS::CommandContext& context, D3D12_RESOURCE_STATES buffersReadyState)
	{
		D_RENDERER_LIGHT::LightContext::UpdateBuffers(context, buffersReadyState);

		auto frameResourceIndex = D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex();

		// Upload shadow data
		ShadowData* mappedShadowData = (ShadowData*)mShadowDataUpload.MapInstance(frameResourceIndex);
		std::memcpy(mappedShadowData, mShadowData.data(), mShadowDataUpload.GetBufferSize());
		mShadowDataUpload.Unmap();
		context.TransitionResource(mShadowDataGpu, D3D12_RESOURCE_STATE_COPY_DEST);
		context.CopyBufferRegion(mShadowDataGpu, 0, mShadowDataUpload, frameResourceIndex * mShadowDataUpload.GetBufferSize(), mShadowDataUpload.GetBufferSize());
		context.TransitionResource(mShadowDataGpu, buffersReadyState);

	}

	void RasterizationShadowedLightContext::Update(D_MATH_CAMERA::Camera const& viewerCamera)
	{
		Reset();


		if ((UINT)mCascades.size() > MaxDirectionalCascades)
			mCascades.resize(MaxDirectionalCascades);
		if ((UINT)mCascades.size() == 0)
			mCascades.resize(1);

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

		mLightConfigBufferData.CascadesCount = GetCascadesCount();
	}

	void RasterizationShadowedLightContext::CalculateDirectionalShadowCamera(D_MATH_CAMERA::Camera const& viewerCamera, LightData& light, int directionalIndex)
	{
		D_CONTAINERS::DVector<D_MATH_CAMERA::Frustum> cascadeFrustums;
		viewerCamera.CalculateSlicedFrustumes(mCascades, cascadeFrustums);

		for (int i = 0; i < (int)cascadeFrustums.size(); i++)
		{
			int resourceIndex = directionalIndex * GetCascadesCount() + i;
			auto const& frustum = cascadeFrustums.at(i);
			auto& cam = mDirectionalShadowCameras[resourceIndex];
			auto lightDir = Vector3(light.Direction);
			cam.SetLookDirection(lightDir, Vector3::Up);
			cam.SetPosition(Vector3(kZero));
			cam.Update();
			auto& shadowView = cam.GetViewMatrix();

			// Finding frustum aabb in shadow space
			auto shadowSpaceFrustum = shadowView * frustum;
			auto shadowSpaceAABB = shadowSpaceFrustum.GetAABB();

			auto dim = shadowSpaceAABB.GetDimensions();
			auto camSPPos = shadowSpaceAABB.GetCenter() + Vector3::Forward * shadowSpaceAABB.GetExtents().GetZ();
			auto camWorldPos = D_MATH::Invert(shadowView) * camSPPos;

			// Fitting directional light bounds to the frustom... With extra Z
			cam.UpdateMatrix(lightDir, Vector3(camWorldPos), dim, mDirectionalShadowBufferWidth, mDirectionalShadowBufferWidth, mShadowBufferDepthPrecision);


			mShadowData[resourceIndex].ShadowMatrix = cam.GetShadowMatrix();
		}
	}

	void RasterizationShadowedLightContext::RenderDirectionalShadow(D_RENDERER_RAST::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData const& light, int directionalIndex)
	{

		GlobalConstants globals;

		int lightIndexStart = directionalIndex * GetCascadesCount();

		for (UINT i = 0u; i < GetCascadesCount(); i++)
		{
			sorter.Reset();

			D_PROFILING::ScopedTimer _prof(std::to_wstring(i), context);
			int resourceIndex = lightIndexStart + i;
			auto const& cam = mDirectionalShadowCameras[resourceIndex];

			globals.ViewProj = cam.GetViewProjMatrix();
			globals.CameraPos = (DirectX::XMFLOAT3)cam.GetPosition();
			auto const& frustum = cam.GetWorldSpaceFrustum();
			for (int i = 0; i < 6; i++)
				globals.FrustumPlanes[i] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)i);

			auto& shadowBuffer = mShadowBuffersDirectional[resourceIndex];
			shadowBuffer.BeginRendering(context);

			sorter.SetCamera(cam);
			sorter.SetDepthStencilTarget(shadowBuffer, nullptr);
			sorter.RenderMeshes(MeshSorter::kZPass, context, nullptr, globals);

			context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

			context.CopySubresource(mDirectionalShadowTextureArrayBuffer, resourceIndex, shadowBuffer, 0);
		}
	}

	void RasterizationShadowedLightContext::CalculateSpotShadowCamera(LightData& light, int spotLightIndex)
	{
		auto& shadowCamera = mSpotShadowCameras[spotLightIndex];
		shadowCamera.SetEyeAtUp(light.Position, Vector3(light.Position) + Vector3(light.Direction), Vector3::Up);
		shadowCamera.SetPerspectiveMatrix(D_MATH::ACos(light.SpotAngles.y) * 2, 1.0f, light.Range * .001f, light.Range * 1.0f);
		shadowCamera.Update();
		mShadowData[spotLightIndex + SpotLightStartIndex].ShadowMatrix = shadowCamera.GetViewProjMatrix();
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
			mShadowData[pointLightIndex + PointLightStartIndex].ShadowMatrix = shadowCamera.GetProjMatrix();
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
		{
			D_PROFILING::ScopedTimer _prof1(L"Directional Shadows", shadowContext);
			for (UINT i = 0u; i < GetNumberOfDirectionalLights(); i++)
			{
				auto const& light = GetDirectionalLightData(i);
				if (!light.CastsShadow)
					continue;

				RenderDirectionalShadow(sorter, shadowContext, light, i);
			}
		}

		// Rendering spot
		{
			D_PROFILING::ScopedTimer _prof1(L"Spont Shadows", shadowContext);

			sorter.SetViewport(D3D12_VIEWPORT());
			sorter.SetScissor({ 1, 1, (long)mSpotShadowBufferWidth - 2l, (long)mSpotShadowBufferWidth - 2l });
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
		}

		// Rendering point
		{
			D_PROFILING::ScopedTimer _prof1(L"Point Shadows", shadowContext);

			sorter.SetViewport(D3D12_VIEWPORT());
			sorter.SetScissor({ 0l, 0l, (long)mPointShadowBufferWidth, (long)mPointShadowBufferWidth });

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
		}

		shadowContext.TransitionResource(mDirectionalShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		shadowContext.TransitionResource(mSpotShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		shadowContext.TransitionResource(mPointShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
	}

	void RasterizationShadowedLightContext::SetCascadeRange(UINT cascadeIndex, float range)
	{
		D_ASSERT(cascadeIndex <= mCascades.size());
		mCascades[cascadeIndex] = range;
	}

	void RasterizationShadowedLightContext::SetCascadesCount(UINT count)
	{
		D_ASSERT(count > 0 && count <= MaxDirectionalCascades);
		mCascades.resize(count - 1, 50u);
		DestroyShadowBuffers();
		CreateShadowBuffers();
	}

	D_CONTAINERS::DVector<float>& RasterizationShadowedLightContext::ModifyCascades()
	{
		return mCascades;
	}

}

