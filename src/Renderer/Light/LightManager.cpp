#include "Renderer/pch.hpp"
#include "LightManager.hpp"

#include "Renderer/Components/LightComponent.hpp"
#include "Renderer/FrameResource.hpp"
#include "Renderer/GraphicsDeviceManager.hpp"
#include "Renderer/GraphicsUtils/Buffers/ShadowBuffer.hpp"
#include "Renderer/GraphicsUtils/Buffers/GpuBuffer.hpp"
#include "Renderer/GraphicsUtils/Buffers/UploadBuffer.hpp"
#include "Renderer/GraphicsUtils/Profiling/Profiling.hpp"
#include "Renderer/Renderer.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Math/Camera/ShadowCamera.hpp>
#include <Job/Job.hpp>
#include <Scene/Scene.hpp>

#include "LightManager.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_GRAPHICS_BUFFERS;
using namespace D_MATH;
using namespace D_RENDERER_FRAME_RESOURCE;
using namespace DirectX;

namespace Darius::Renderer::LightManager
{

	struct LightStatus
	{
		uint16_t LightActive : 1;
		uint16_t ComponentActive : 1;
	};

	bool								_initialized = false;

	DVector<LightData>					DirectionalLights;
	DVector<LightData>					PointLights;
	DVector<LightData>					SpotLights;

	DVector<D_GRAPHICS_BUFFERS::ShadowBuffer> ShadowBuffersDirectional;
	DVector<D_GRAPHICS_BUFFERS::ShadowBuffer> ShadowBuffersSpot;
	DVector<D_GRAPHICS_BUFFERS::ShadowBuffer> ShadowBuffersPoint;
	D_GRAPHICS_BUFFERS::ColorBuffer		DirectionalShadowTextureArrayBuffer;
	D_GRAPHICS_BUFFERS::ColorBuffer		PointShadowTextureArrayBuffer;
	D_GRAPHICS_BUFFERS::ColorBuffer		SpotShadowTextureArrayBuffer;

	DVector<D_MATH_CAMERA::ShadowCamera> DirectionalShadowCameras;
	DVector<D_MATH_CAMERA::Camera>		SpotShadowCameras;
	DVector<D_MATH_CAMERA::Camera>		PointShadowCameras;

	// Gpu buffers
	D_GRAPHICS_BUFFERS::UploadBuffer	ActiveLightsUpload;
	D_GRAPHICS_BUFFERS::UploadBuffer	LightsUpload;
	ByteAddressBuffer					ActiveLightsBufferGpu;
	StructuredBuffer					LightsBufferGpu;

	/////////////////////////////////////////////////
	// Options

	uint32_t							DirectionalShadowBufferWidth = 4096;
	uint32_t							PointShadowBufferWidth = 512;
	uint32_t							SpotShadowBufferWidth = 1024;
	uint32_t							ShadowBufferDepthPercision = 16;

	void CalculateSpotShadowCamera(LightData& light, int spotLightIndex);
	void CalculateDirectionalShadowCamera(D_MATH_CAMERA::Camera const& viewerCamera, LightData& light, int directionalIndex);
	void CalculatePointShadowCamera(LightData& light, int pointLightIndex);

	void Initialize()
	{
		D_ASSERT(!_initialized);

		// Initializing buffers
		size_t elemSize = sizeof(UINT) * 8;
		size_t count = (MaxNumLight + elemSize - 1) / elemSize;
		ActiveLightsUpload.Create(L"Active Light Upload", count * sizeof(UINT));
		LightsUpload.Create(L"Light Upload", MaxNumLight * sizeof(LightData));
		ActiveLightsBufferGpu.Create(L"Active Light Buffer", (UINT)count, sizeof(UINT), nullptr);
		LightsBufferGpu.Create(L"Light Buffer", MaxNumLight, sizeof(LightData));

		ShadowBuffersDirectional.resize(MaxNumDirectionalLight);
		ShadowBuffersPoint.resize(MaxNumPointLight * 6);
		ShadowBuffersSpot.resize(MaxNumSpotLight);

		for (int i = 0; i < ShadowBuffersDirectional.size(); i++)
			ShadowBuffersDirectional[i].Create(std::wstring(L"Directional Shadow Buffer ") + std::to_wstring(i), DirectionalShadowBufferWidth, DirectionalShadowBufferWidth, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

		for (int i = 0; i < ShadowBuffersPoint.size(); i++)
			ShadowBuffersPoint[i].Create(std::wstring(L"Point Shadow Buffer ") + std::to_wstring(i % 6) + L"/6 " + std::to_wstring(i / 6), PointShadowBufferWidth, PointShadowBufferWidth, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
		for (int i = 0; i < ShadowBuffersSpot.size(); i++)
			ShadowBuffersSpot[i].Create(std::wstring(L"Spot Shadow Buffer ") + std::to_wstring(i), SpotShadowBufferWidth, SpotShadowBufferWidth, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

		// Initializing main shadow buffers
		auto shadowBufferFormat = D_GRAPHICS::GetShadowFormat();
		DirectionalShadowTextureArrayBuffer.CreateArray(L"Directional Shadow Texture Array Buffer", DirectionalShadowBufferWidth, DirectionalShadowBufferWidth, MaxNumDirectionalLight, DXGI_FORMAT_R16_UNORM);
		PointShadowTextureArrayBuffer.CreateArray(L"Point Shadow Texture Array Buffer", PointShadowBufferWidth, PointShadowBufferWidth, 6 * MaxNumPointLight, DXGI_FORMAT_R16_UNORM, true);
		SpotShadowTextureArrayBuffer.CreateArray(L"Spot Shadow Texture Array Buffer", SpotShadowBufferWidth, SpotShadowBufferWidth, MaxNumSpotLight, DXGI_FORMAT_R16_UNORM);

		// Create shadow cameras
		DirectionalShadowCameras.resize(MaxNumDirectionalLight);
		SpotShadowCameras.resize(MaxNumSpotLight);
		PointShadowCameras.resize(MaxNumPointLight * 6);
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		DirectionalShadowTextureArrayBuffer.Destroy();
		PointShadowTextureArrayBuffer.Destroy();
		SpotShadowTextureArrayBuffer.Destroy();

		for (int i = 0; i < ShadowBuffersDirectional.size(); i++)
			ShadowBuffersDirectional[i].Destroy();
		for (int i = 0; i < ShadowBuffersPoint.size(); i++)
			ShadowBuffersPoint[i].Destroy();
		for (int i = 0; i < ShadowBuffersSpot.size(); i++)
			ShadowBuffersSpot[i].Destroy();

		ActiveLightsUpload.Destroy();
		LightsUpload.Destroy();
		ActiveLightsBufferGpu.Destroy();
		LightsBufferGpu.Destroy();
	}

	void Update()
	{
		DirectionalLights.clear();
		PointLights.clear();
		SpotLights.clear();

		auto& reg = D_WORLD::GetRegistry();
		reg.each([](D_GRAPHICS::LightComponent& comp)
			{
				if (!comp.IsActive())
					return;

				auto lightData = comp.GetLightData();
				auto trans = comp.GetTransform();
				lightData.Position = (DirectX::XMFLOAT3)trans->GetPosition();
				lightData.Direction = (DirectX::XMFLOAT3)trans->GetRotation().GetForward();

				switch (comp.GetLightType())
				{
				case D_LIGHT::LightSourceType::DirectionalLight:
					DirectionalLights.push_back(lightData);
					break;
				case D_LIGHT::LightSourceType::PointLight:
					PointLights.push_back(lightData);
					break;
				case D_LIGHT::LightSourceType::SpotLight:
					SpotLights.push_back(lightData);
					break;
				}
			});
	}

	void UpdateBuffers(D_GRAPHICS::GraphicsContext& context, D_MATH_CAMERA::Camera const* viewerCamera)
	{
		// Upload buffers state
		UINT* data = (UINT*)ActiveLightsUpload.Map();
		LightData* lightUploadData = (LightData*)LightsUpload.Map();
		LightSourceType lightSource;

		bool done = false;

		for (UINT i = 0; !done && i < (MaxNumLight + sizeof(UINT) * 8 - 1) / sizeof(UINT) * 8; i++)
		{
			UINT activeFlags = 0;

			for (short bitIdx = 0; bitIdx < sizeof(UINT) * 8; bitIdx++)
			{
				UINT lightIdx = i * (UINT)sizeof(UINT) * 8u + bitIdx;
				DVector<LightData>* LightVec = nullptr;
				int indexInVec = -1;

				// Setting light status
				bool lightStatus = false;
				if (lightIdx < MaxNumDirectionalLight)
				{
					indexInVec = lightIdx;
					LightVec = &DirectionalLights;
					lightSource = LightSourceType::DirectionalLight;
				}
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight)
				{
					indexInVec = lightIdx - MaxNumDirectionalLight;
					LightVec = &PointLights;
					lightSource = LightSourceType::PointLight;
				}
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight + MaxNumSpotLight)
				{
					indexInVec = lightIdx - MaxNumDirectionalLight - MaxNumPointLight;
					LightVec = &SpotLights;
					lightSource = LightSourceType::SpotLight;
				}
				else
				{
					done = true;
					break;
				}

				lightStatus = indexInVec < LightVec->size();

				if (lightStatus)
				{
					// Setting location and direction
					LightData& lightData = (*LightVec)[indexInVec];

					switch (lightSource)
					{
					case Darius::Renderer::LightManager::LightSourceType::SpotLight:
						CalculateSpotShadowCamera(lightData, indexInVec);
						break;
					case Darius::Renderer::LightManager::LightSourceType::PointLight:
						CalculatePointShadowCamera(lightData, indexInVec);
						break;
					case Darius::Renderer::LightManager::LightSourceType::DirectionalLight:
						if (viewerCamera)
							CalculateDirectionalShadowCamera(*viewerCamera, lightData, indexInVec);
						break;
					default:
						break;
					}

					activeFlags += 1;

					lightUploadData[lightIdx] = lightData;
				}
				if (bitIdx < 31)
					activeFlags <<= 1;
			}
			data[i] = activeFlags;
		}

		ActiveLightsUpload.Unmap();
		LightsUpload.Unmap();

		// Uploading...

		context.PIXBeginEvent(L"Uploading light masks and data");

		context.TransitionResource(LightsBufferGpu, D3D12_RESOURCE_STATE_COPY_DEST);
		context.TransitionResource(ActiveLightsBufferGpu, D3D12_RESOURCE_STATE_COPY_DEST, true);

		context.CopyBufferRegion(LightsBufferGpu, 0, LightsUpload, 0, LightsUpload.GetBufferSize());
		context.CopyBufferRegion(ActiveLightsBufferGpu, 0, ActiveLightsUpload, 0, ActiveLightsUpload.GetBufferSize());

		context.TransitionResource(LightsBufferGpu, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		context.TransitionResource(ActiveLightsBufferGpu, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

		context.PIXEndEvent();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetLightMaskHandle()
	{
		return ActiveLightsBufferGpu.GetSRV();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetLightDataHandle()
	{
		return LightsBufferGpu.GetSRV();
	}

	void GetShadowTextureArrayHandle(D3D12_CPU_DESCRIPTOR_HANDLE& directional, D3D12_CPU_DESCRIPTOR_HANDLE& point, D3D12_CPU_DESCRIPTOR_HANDLE& spot)
	{
		directional = DirectionalShadowTextureArrayBuffer.GetSRV();
		point = PointShadowTextureArrayBuffer.GetSRV();
		spot = SpotShadowTextureArrayBuffer.GetSRV();
	}

	void CalculateDirectionalShadowCamera(D_MATH_CAMERA::Camera const& viewerCamera, LightData& light, int directionalIndex)
	{
		auto& cam = DirectionalShadowCameras[directionalIndex];
		auto lightDir = Vector3(light.Direction);
		cam.SetLookDirection(lightDir, Vector3::Up);
		cam.SetPosition(Vector3(kZero));
		cam.Update();
		auto shadowView = cam.GetViewMatrix();

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
		cam.UpdateMatrix(lightDir, Vector3(camWorldPos), dim, DirectionalShadowBufferWidth, DirectionalShadowBufferWidth, ShadowBufferDepthPercision);

		light.ShadowMatrix = cam.GetShadowMatrix();
	}

	void RenderDirectionalShadow(D_MATH_CAMERA::Camera const& viewerCamera, D_RENDERER::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData const& light, int directionalIndex)
	{
		auto const& cam = DirectionalShadowCameras[directionalIndex];

		GlobalConstants globals;
		globals.ViewProj = cam.GetViewProjMatrix();
		globals.CameraPos = (DirectX::XMFLOAT3)cam.GetPosition();
		globals.ShadowTexelSize.x = 1.f / DirectionalShadowBufferWidth;
		auto const& frustum = cam.GetWorldSpaceFrustum();
		for (int i = 0; i < 6; i++)
			globals.FrustumPlanes[i] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)i);

		auto& shadowBuffer = ShadowBuffersDirectional[directionalIndex];
		shadowBuffer.BeginRendering(context);

		sorter.SetCamera(cam);
		sorter.SetDepthStencilTarget(shadowBuffer);
		sorter.RenderMeshes(MeshSorter::kZPass, context, nullptr, globals);

		context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		context.CopySubresource(DirectionalShadowTextureArrayBuffer, directionalIndex, shadowBuffer, 0);
	}

	void CalculateSpotShadowCamera(LightData& light, int spotLightIndex)
	{
		auto& shadowCamera = SpotShadowCameras[spotLightIndex];
		shadowCamera.SetEyeAtUp(light.Position, Vector3(light.Position) + Vector3(light.Direction), Vector3::Up);
		shadowCamera.SetPerspectiveMatrix(D_MATH::ACos(light.SpotAngles.y) * 2, 1.0f, light.Range * .001f, light.Range * 1.0f);
		shadowCamera.Update();
		light.ShadowMatrix = shadowCamera.GetViewProjMatrix();
	}

	void RenderSpotShadow(D_RENDERER::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData const& light, int spotLightIndex)
	{
		auto const& shadowCamera = SpotShadowCameras[spotLightIndex];

		GlobalConstants globals;
		globals.ViewProj = shadowCamera.GetViewProjMatrix();
		globals.CameraPos = (DirectX::XMFLOAT3)shadowCamera.GetPosition();
		auto const& frustum = shadowCamera.GetWorldSpaceFrustum();
		for (int i = 0; i < 6; i++)
			globals.FrustumPlanes[i] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)i);

		auto& shadowBuffer = ShadowBuffersSpot[spotLightIndex];

		shadowBuffer.BeginRendering(context);

		sorter.SetCamera(shadowCamera);
		sorter.SetDepthStencilTarget(shadowBuffer);
		sorter.RenderMeshes(MeshSorter::kZPass, context, nullptr, globals);

		context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		context.CopySubresource(SpotShadowTextureArrayBuffer, spotLightIndex, shadowBuffer, 0);
	}

	void CalculatePointShadowCamera(LightData& light, int pointLightIndex)
	{
		static const float nearDist = 0.05f;
		// Left
		{
			auto& shadowCamera = PointShadowCameras[pointLightIndex * 6];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Left, Vector3::Up);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
			light.ShadowMatrix = shadowCamera.GetProjMatrix();
		}

		// Right
		{
			auto& shadowCamera = PointShadowCameras[pointLightIndex * 6 + 1];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Right, Vector3::Up);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

		// Top
		{
			auto& shadowCamera = PointShadowCameras[pointLightIndex * 6 + 2];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Up, Vector3::Forward);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

		// Bottom
		{
			auto& shadowCamera = PointShadowCameras[pointLightIndex * 6 + 3];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Down, Vector3::Backward);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

		// Back
		{
			auto& shadowCamera = PointShadowCameras[pointLightIndex * 6 + 4];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Backward, Vector3::Up);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

		// Front
		{
			auto& shadowCamera = PointShadowCameras[pointLightIndex * 6 + 5];
			shadowCamera.SetPosition(light.Position);
			shadowCamera.SetLookDirection(Vector3::Forward, Vector3::Up);
			shadowCamera.SetPerspectiveMatrix(DirectX::XM_PIDIV2, 1.0f, nearDist, light.Range);
			shadowCamera.Update();
		}

	}

	void RenderPointShadow(D_RENDERER::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData const& light, int pointLightIndex)
	{
		GlobalConstants globals;
		{
			auto& refCam = PointShadowCameras[pointLightIndex * 6];
			globals.CameraPos = (DirectX::XMFLOAT3)refCam.GetPosition();
			globals.Proj = refCam.GetProjMatrix();
		}

		for (int i = 0; i < 6; i++)
		{
			auto const& shadowCamera = PointShadowCameras[pointLightIndex * 6 + i];

			globals.ViewProj = shadowCamera.GetViewProjMatrix();
			auto const& frustum = shadowCamera.GetWorldSpaceFrustum();
			for (int j = 0; j < 6; j++)
				globals.FrustumPlanes[j] = (Vector4)frustum.GetFrustumPlane((D_MATH_CAMERA::Frustum::PlaneID)j);

			auto& shadowBuffer = ShadowBuffersPoint[pointLightIndex * 6 + i];

			shadowBuffer.BeginRendering(context);
			sorter.Reset();
			sorter.SetCamera(shadowCamera);
			sorter.SetDepthStencilTarget(shadowBuffer);
			sorter.RenderMeshes(MeshSorter::kZPass, context, nullptr, globals);
			context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

			context.CopySubresource(PointShadowTextureArrayBuffer, pointLightIndex * 6 + i, shadowBuffer, 0);
		}
	}

	void RenderShadows(D_MATH_CAMERA::Camera const& viewerCamera, D_CONTAINERS::DVector<RenderItem> const& shadowRenderItems, D_GRAPHICS::GraphicsContext& shadowContext)
	{

		D_PROFILING::ScopedTimer _prof(L"Rendering Shadows", shadowContext);

		// TODO: Create sorter and input render item per light source
		MeshSorter sorter(MeshSorter::kShadows);
		for (auto const& ri : shadowRenderItems) sorter.AddMesh(ri, 0.1f);

		sorter.Sort();

		shadowContext.TransitionResource(DirectionalShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
		shadowContext.TransitionResource(SpotShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
		shadowContext.TransitionResource(PointShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);

		// Rendering directional
		for (int i = 0; i < DirectionalLights.size(); i++)
		{

			sorter.Reset();

			auto const& light = DirectionalLights[i];
			if (!light.CastsShadow)
				continue;

			RenderDirectionalShadow(viewerCamera, sorter, shadowContext, light, i);
		}
		sorter.SetViewport(D3D12_VIEWPORT());
		sorter.SetScissor({ 1, 1, (long)SpotShadowBufferWidth - 2l, (long)SpotShadowBufferWidth - 2l });

		// Rendering spot
		for (int idx = 0; idx < MaxNumSpotLight; idx++)
		{
			if (idx >= SpotLights.size())
				break;

			sorter.Reset();
			auto const& light = SpotLights[idx];
			if (!light.CastsShadow)
				continue;

			RenderSpotShadow(sorter, shadowContext, light, idx);
		}
		sorter.SetViewport(D3D12_VIEWPORT());
		sorter.SetScissor({ 0l, 0l, (long)PointShadowBufferWidth, (long)PointShadowBufferWidth });

		// Rendering point
		for (int idx = 0; idx < MaxNumPointLight; idx++)
		{
			if (idx >= PointLights.size())
				break;

			sorter.Reset();
			auto const& light = PointLights[idx];
			if (!light.CastsShadow)
				continue;

			RenderPointShadow(sorter, shadowContext, light, idx);
		}

		shadowContext.TransitionResource(DirectionalShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		shadowContext.TransitionResource(SpotShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		shadowContext.TransitionResource(PointShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

	}

}

