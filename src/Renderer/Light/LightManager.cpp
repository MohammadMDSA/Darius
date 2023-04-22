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

	DVector<LightStatus>				ActiveDirectionalLight;
	DVector<LightStatus>				ActivePointLight;
	DVector<LightStatus>				ActiveSpotLight;

	DVector<Transform>					DirectionalLightTransforms;
	DVector<Transform>					PointLightTransforms;
	DVector<Transform>					SpotLightTransforms;

	DVector<D_GRAPHICS_BUFFERS::ShadowBuffer> ShadowBuffers;
	D_GRAPHICS_BUFFERS::ColorBuffer		DirectionalShadowTextureArrayBuffer;
	D_GRAPHICS_BUFFERS::ColorBuffer		PointShadowTextureArrayBuffer;
	D_GRAPHICS_BUFFERS::ColorBuffer		SpotShadowTextureArrayBuffer;

	DVector<D_MATH_CAMERA::ShadowCamera> DirectionalShadowCameras;
	DVector<D_MATH_CAMERA::Camera>		SpotShadowCameras;

	// Gpu buffers
	D_GRAPHICS_BUFFERS::UploadBuffer	ActiveLightsUpload[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
	D_GRAPHICS_BUFFERS::UploadBuffer	LightsUpload[D_RENDERER_FRAME_RESOURCE::gNumFrameResources];
	ByteAddressBuffer					ActiveLightsBufferGpu;
	StructuredBuffer					LightsBufferGpu;

	INLINE DVector<LightStatus>* GetAssociatedActiveLightWithType(LightSourceType type);
	INLINE DVector<LightData>* GetAssociatedLightsWithType(LightSourceType type);
	INLINE DVector<Transform>* GetAssociatedLightTransformsWithType(LightSourceType type);


	/////////////////////////////////////////////////
	// Options

	uint32_t							DirectionalShadowBufferWidth = 4096;
	uint32_t							PointShadowBufferWidth = 512;
	uint32_t							SpotShadowBufferWidth = 1024;
	uint32_t							ShadowBufferDepthPercision = 16;

	void CalculateSpotShadowCamera(LightData& light, int lightGloablIndex);
	void CalculateDirectionalShadowCamera(D_MATH_CAMERA::Camera const& viewerCamera, LightData& light, int lightGloablIndex);


	void Initialize()
	{
		D_ASSERT(!_initialized);
		DirectionalLights.resize(MaxNumDirectionalLight);
		ActiveDirectionalLight.resize(MaxNumDirectionalLight);
		DirectionalLightTransforms.resize(MaxNumDirectionalLight);
		PointLights.resize(MaxNumPointLight);
		ActivePointLight.resize(MaxNumPointLight);
		PointLightTransforms.resize(MaxNumPointLight);
		SpotLights.resize(MaxNumSpotLight);
		ActiveSpotLight.resize(MaxNumSpotLight);
		SpotLightTransforms.resize(MaxNumSpotLight);

		Reset();

		// Initializing buffers
		size_t elemSize = sizeof(UINT) * 8;
		size_t count = (MaxNumLight + elemSize - 1) / elemSize;
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOURCE::gNumFrameResources; i++)
		{
			ActiveLightsUpload[i].Create(L"Active Light Upload", count * sizeof(UINT));
			LightsUpload[i].Create(L"Light Upload", MaxNumLight * sizeof(LightData));
		}
		ActiveLightsBufferGpu.Create(L"Active Light Buffer", (UINT)count, sizeof(UINT), nullptr);
		LightsBufferGpu.Create(L"Light Buffer", MaxNumLight, sizeof(LightData));

		ShadowBuffers.resize(MaxNumLight);
		for (int i = 0; i < ShadowBuffers.size(); i++)
		{
			UINT width = 0u;
			if (i < MaxNumDirectionalLight)
				width = DirectionalShadowBufferWidth;
			else if (i < MaxNumDirectionalLight + MaxNumPointLight)
				width = PointShadowBufferWidth;
			else
				width = SpotShadowBufferWidth;

			ShadowBuffers[i].Create(std::wstring(L"Shadow Buffer ") + std::to_wstring(i), width, width, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
		}
		DirectionalShadowTextureArrayBuffer.CreateArray(L"Directional Shadow Texture Array Buffer", DirectionalShadowBufferWidth, DirectionalShadowBufferWidth, MaxNumDirectionalLight, DXGI_FORMAT_R16_UNORM);
		PointShadowTextureArrayBuffer.CreateArray(L"Point Shadow Texture Array Buffer", PointShadowBufferWidth, PointShadowBufferWidth, MaxNumPointLight, DXGI_FORMAT_R16_UNORM);
		SpotShadowTextureArrayBuffer.CreateArray(L"Spot Shadow Texture Array Buffer", SpotShadowBufferWidth, SpotShadowBufferWidth, MaxNumSpotLight, DXGI_FORMAT_R16_UNORM);

		// Create shadow cameras
		DirectionalShadowCameras.resize(MaxNumDirectionalLight);
		SpotShadowCameras.resize(MaxNumSpotLight);
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

	void Reset()
	{

		for (size_t i = 0; i < MaxNumDirectionalLight; i++)
		{
			ActiveDirectionalLight[i] = { false, false };
		}

		for (size_t i = 0; i < MaxNumPointLight; i++)
		{
			ActivePointLight[i] = { false, false };
		}

		for (size_t i = 0; i < MaxNumSpotLight; i++)
		{
			ActiveSpotLight[i] = { false, false };
		}
	}

	void Update()
	{
		auto& reg = D_WORLD::GetRegistry();
		reg.each([](D_GRAPHICS::LightComponent& comp)
			{
				comp.Update(-1);
			});
	}

	void UpdateBuffers(D_GRAPHICS::GraphicsContext& context, D_MATH_CAMERA::Camera const* viewrCamera)
	{
		auto& currentActiveLightUpload = ActiveLightsUpload[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];
		auto& currentLightUpload = LightsUpload[D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex()];

		// Upload buffers state
		UINT* data = (UINT*)currentActiveLightUpload.Map();
		LightData* lightUploadData = (LightData*)currentLightUpload.Map();
		LightSourceType lightSource;

		bool done = false;

		for (UINT i = 0; !done && i < (MaxNumLight + sizeof(UINT) * 8 - 1) / sizeof(UINT) * 8; i++)
		{
			UINT activeFlags = 0;

			for (short bitIdx = 0; bitIdx < sizeof(UINT) * 8; bitIdx++)
			{
				UINT lightIdx = i * (UINT)sizeof(UINT) * 8u + bitIdx;
				DVector<LightData>* LightVec = nullptr;
				DVector<Transform>* transformVec = nullptr;
				DVector<LightStatus>* activeVec = nullptr;
				int indexInVec = -1;

				// Setting light status
				bool lightStatus = false;
				if (lightIdx < MaxNumDirectionalLight)
				{
					indexInVec = lightIdx;
					LightVec = &DirectionalLights;
					transformVec = &DirectionalLightTransforms;
					activeVec = &ActiveDirectionalLight;
					lightSource = LightSourceType::DirectionalLight;
				}
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight)
				{
					indexInVec = lightIdx - MaxNumDirectionalLight;
					LightVec = &PointLights;
					transformVec = &PointLightTransforms;
					activeVec = &ActivePointLight;
					lightSource = LightSourceType::PointLight;
				}
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight + MaxNumSpotLight)
				{
					indexInVec = lightIdx - MaxNumDirectionalLight - MaxNumPointLight;
					LightVec = &SpotLights;
					transformVec = &SpotLightTransforms;
					activeVec = &ActiveSpotLight;
					lightSource = LightSourceType::SpotLight;
				}
				else
				{
					done = true;
					break;
				}

				lightStatus = (*activeVec)[indexInVec].LightActive && (*activeVec)[indexInVec].ComponentActive;

				if (lightStatus)
				{
					// Setting location and direction
					auto trans = (*transformVec)[indexInVec];
					LightData& lightData = (*LightVec)[indexInVec];
					lightData.Position = (XMFLOAT3)trans.Translation;
					XMStoreFloat3(&lightData.Direction, XMVector3Rotate({ 0.f, 0.f, -1.f }, trans.Rotation));

					switch (lightSource)
					{
					case Darius::Renderer::LightManager::LightSourceType::SpotLight:
						CalculateSpotShadowCamera(lightData, lightIdx);
						break;
					case Darius::Renderer::LightManager::LightSourceType::DirectionalLight:
						if (viewrCamera)
							CalculateDirectionalShadowCamera(*viewrCamera, lightData, lightIdx);
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

		currentActiveLightUpload.Unmap();
		currentLightUpload.Unmap();

		// Uploading...

		context.PIXBeginEvent(L"Uploading light masks and data");

		context.TransitionResource(LightsBufferGpu, D3D12_RESOURCE_STATE_COPY_DEST);
		context.TransitionResource(ActiveLightsBufferGpu, D3D12_RESOURCE_STATE_COPY_DEST, true);

		context.CopyBufferRegion(LightsBufferGpu, 0, currentLightUpload, 0, currentLightUpload.GetBufferSize());
		context.CopyBufferRegion(ActiveLightsBufferGpu, 0, currentActiveLightUpload, 0, currentActiveLightUpload.GetBufferSize());

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

	INLINE DVector<LightStatus>* GetAssociatedActiveLightWithType(LightSourceType type)
	{
		switch (type)
		{
		case LightSourceType::DirectionalLight:
			return &ActiveDirectionalLight;
		case LightSourceType::PointLight:
			return &ActivePointLight;
		case LightSourceType::SpotLight:
			return &ActiveSpotLight;
		}
		return nullptr;
	}

	INLINE DVector<Transform>* GetAssociatedLightTransformsWithType(LightSourceType type)
	{
		switch (type)
		{
		case LightSourceType::DirectionalLight:
			return &DirectionalLightTransforms;
		case LightSourceType::PointLight:
			return &PointLightTransforms;
		case LightSourceType::SpotLight:
			return &SpotLightTransforms;
		}

		return nullptr;
	}

	INLINE DVector<LightData>* GetAssociatedLightsWithType(LightSourceType type)
	{
		switch (type)
		{
		case LightSourceType::DirectionalLight:
			return &DirectionalLights;
		case LightSourceType::PointLight:
			return &PointLights;
		case LightSourceType::SpotLight:
			return &SpotLights;
		}
		return nullptr;
	}

	int AccuireLightSource(LightSourceType type)
	{
		auto typeOwners = GetAssociatedActiveLightWithType(type);

		// Finding an index in light sources array that is no one is using
		int emptyIndex = -1;
		for (int i = 0; i < typeOwners->size(); i++)
		{
			if (!typeOwners->at(i).LightActive)
			{
				emptyIndex = i;
				break;
			}
		}

		// No light source of specified type is available
		if (emptyIndex == -1)
			return -1;

		(*typeOwners)[emptyIndex].LightActive = true;

		return emptyIndex;
	}

	int SwapLightSource(LightSourceType type, LightSourceType preType, int preIndex)
	{

		auto preTypeLights = GetAssociatedLightsWithType(preType);

		// Requesting the same type, so return what requester already own
		if (preType == type)
			return preIndex;

		// Removing owner data to accuire new light
		ReleaseLight(preType, preIndex);

		auto newLight = AccuireLightSource(type);

		return newLight;
	}

	void ReleaseLight(LightSourceType preType, int preIndex)
	{
		(*GetAssociatedActiveLightWithType(preType))[preIndex].LightActive = false;
	}

	INLINE int GetGlobalLightIndex(LightSourceType type, int typeIndex)
	{
		switch (type)
		{
		case Darius::Renderer::LightManager::LightSourceType::DirectionalLight:
			return typeIndex;
		case Darius::Renderer::LightManager::LightSourceType::PointLight:
			return MaxNumDirectionalLight + typeIndex;
		case Darius::Renderer::LightManager::LightSourceType::SpotLight:
			return MaxNumDirectionalLight + MaxNumPointLight + typeIndex;
		default:
			return -1;
		}
	}

	void CalculateDirectionalShadowCamera(D_MATH_CAMERA::Camera const& viewerCamera, LightData& light, int lightGloablIndex)
	{
		auto& cam = DirectionalShadowCameras[lightGloablIndex];
		auto lightDir = Vector3(light.Direction);
		cam.SetLookDirection(lightDir, Vector3::Up);
		cam.SetPosition(Vector3(kZero));
		cam.Update();
		auto shadowView = cam.GetViewMatrix();

		auto const& viewrFrustom = viewerCamera.GetWorldSpaceFrustum();
		auto shadowSpaceAABB = D_MATH_BOUNDS::AxisAlignedBox();

		for (int i = 0; i < D_MATH_CAMERA::Frustum::_kNumCorners; i++)
		{
			auto corner = viewrFrustom.GetFrustumCorner((D_MATH_CAMERA::Frustum::CornerID)i);
			auto shadowSpaceCorner = shadowView * corner;
			shadowSpaceAABB.AddPoint(Vector3(shadowSpaceCorner));
		}

		auto camWorldPos = D_MATH::Invert(shadowView) * shadowSpaceAABB.GetCenter();
		auto dim = shadowSpaceAABB.GetDimensions();

		cam.UpdateMatrix(lightDir, Vector3(camWorldPos), shadowSpaceAABB.GetDimensions(), DirectionalShadowBufferWidth, DirectionalShadowBufferWidth, ShadowBufferDepthPercision);

		light.ShadowMatrix = cam.GetShadowMatrix();
	}

	void RenderDirectionalShadow(D_MATH_CAMERA::Camera const& viewerCamera, D_RENDERER::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData const& light, int lightGloablIndex)
	{
		auto const& cam = DirectionalShadowCameras[lightGloablIndex];

		GlobalConstants globals;
		globals.ViewProj = cam.GetViewProjMatrix();
		globals.ShadowTexelSize.x = 1.f / DirectionalShadowBufferWidth;

		auto& shadowBuffer = ShadowBuffers[lightGloablIndex];
		shadowBuffer.BeginRendering(context);

		sorter.SetCamera(cam);
		sorter.SetDepthStencilTarget(shadowBuffer);
		sorter.RenderMeshes(MeshSorter::kZPass, context, nullptr, globals);

		context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		context.TransitionResource(DirectionalShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);

		context.CopySubresource(DirectionalShadowTextureArrayBuffer, lightGloablIndex, shadowBuffer, 0);
	}

	void CalculateSpotShadowCamera(LightData& light, int lightGloablIndex)
	{
		auto& shadowCamera = SpotShadowCameras[lightGloablIndex - MaxNumDirectionalLight - MaxNumPointLight];
		shadowCamera.SetEyeAtUp(light.Position, Vector3(light.Position) + Vector3(light.Direction), Vector3(0, 1, 0));
		shadowCamera.SetPerspectiveMatrix(D_MATH::ACos(light.SpotAngles.y) * 2, 1.0f, light.Range * .001f, light.Range * 1.0f);
		shadowCamera.Update();
		light.ShadowMatrix = shadowCamera.GetViewProjMatrix();
	}

	void RenderSpotShadow(D_RENDERER::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData const& light, int lightGloablIndex)
	{
		auto const& shadowCamera = SpotShadowCameras[lightGloablIndex - MaxNumDirectionalLight - MaxNumPointLight];

		GlobalConstants globals;
		globals.ViewProj = shadowCamera.GetViewProjMatrix();

		auto& shadowBuffer = ShadowBuffers[lightGloablIndex];

		shadowBuffer.BeginRendering(context);

		sorter.SetCamera(shadowCamera);
		sorter.SetDepthStencilTarget(shadowBuffer);
		sorter.RenderMeshes(MeshSorter::kZPass, context, nullptr, globals);

		context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		context.TransitionResource(SpotShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);

		context.CopySubresource(SpotShadowTextureArrayBuffer, lightGloablIndex - MaxNumDirectionalLight - MaxNumPointLight, shadowBuffer, 0);

	}

	void RenderShadows(D_MATH_CAMERA::Camera const& viewerCamera, D_CONTAINERS::DVector<RenderItem> const& shadowRenderItems, D_GRAPHICS::GraphicsContext& shadowContext)
	{

		D_PROFILING::ScopedTimer _prof(L"Rendering Shadows", shadowContext);

		// TODO: Create sorter and input render item per light source
		MeshSorter sorter(MeshSorter::kShadows);
		for (auto const& ri : shadowRenderItems) sorter.AddMesh(ri, 0.1f);

		sorter.Sort();

		for (int i = 0; i < MaxNumDirectionalLight; i++)
		{

			//D_JOB::AssignTask([&](int, int) {

			sorter.Reset();

			auto const& light = DirectionalLights[i];
			if (!light.CastsShadow || !ActiveDirectionalLight[i].LightActive || !ActiveDirectionalLight[i].ComponentActive)
				continue;
			RenderDirectionalShadow(viewerCamera, sorter, shadowContext, light, i);
		}
		sorter.SetViewport(D3D12_VIEWPORT());
		sorter.SetScissor({ 1, 1, (long)SpotShadowBufferWidth - 2l, (long)SpotShadowBufferWidth - 2l });
		for (int i = MaxNumDirectionalLight + MaxNumPointLight; i < MaxNumLight; i++)
		{
			sorter.Reset();
			auto idx = i - (MaxNumPointLight + MaxNumDirectionalLight);
			auto const& light = SpotLights[idx];
			if (!light.CastsShadow || !ActiveSpotLight[idx].LightActive || !ActiveSpotLight[idx].ComponentActive)
				continue;
			RenderSpotShadow(sorter, shadowContext, light, i);
		}
		//});
		shadowContext.TransitionResource(DirectionalShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		shadowContext.TransitionResource(SpotShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		shadowContext.TransitionResource(PointShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

		//if (D_JOB::IsMainThread())
		//	D_JOB::WaitForThreadsToFinish();
	}

	void UpdateLight(LightSourceType type, int index, Transform const& trans, bool active, LightData const& light)
	{
		auto& lightStat = GetAssociatedActiveLightWithType(type)->at(index);
		lightStat.ComponentActive = active;

		if (!active)
			return;

		GetAssociatedLightTransformsWithType(type)->at(index) = trans;

		auto preLight = GetAssociatedLightsWithType(type)->at(index);
		LightData newLight = light;
		newLight.ShadowMatrix = preLight.ShadowMatrix;
		GetAssociatedLightsWithType(type)->at(index) = newLight;

	}

}

