#include "pch.hpp"
#include "LightManager.hpp"

#include "FrameResource.hpp"
#include "GraphicsUtils/Buffers/ShadowBuffer.hpp"
#include "GraphicsUtils/Buffers/GpuBuffer.hpp"
#include "GraphicsUtils/Buffers/UploadBuffer.hpp"
#include "RenderDeviceManager.hpp"
#include "Renderer/Renderer.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/Exceptions/Exception.hpp>
#include <Math/Camera/ShadowCamera.hpp>
#include <Job/Job.hpp>

using namespace D_GRAPHICS_BUFFERS;
using namespace D_CONTAINERS;
using namespace D_MATH;

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
	D_GRAPHICS_BUFFERS::ColorBuffer		ShadowTextureArrayBuffer;

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

	uint32_t							ShadowBufferWidth = 512;
	uint32_t							ShodowBufferHeight = 512;
	uint32_t							ShadowBufferDepthPercision = 16;

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
		ActiveLightsBufferGpu.Create(L"Active Light Buffer", count, sizeof(UINT), nullptr);
		LightsBufferGpu.Create(L"Light Buffer", MaxNumLight, sizeof(LightData));

		ShadowBuffers.resize(MaxNumLight);
		for (int i = 0; i < ShadowBuffers.size(); i++)
		{
			ShadowBuffers[i].Create(std::wstring(L"Shadow Buffer " + i), ShadowBufferWidth, ShodowBufferHeight, D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);
		}
		ShadowTextureArrayBuffer.CreateArray(L"Shadow Texture Array Buffer", ShadowBufferWidth, ShodowBufferHeight, MaxNumLight, DXGI_FORMAT_R16_UNORM);

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

	void UpdateBuffers(D_GRAPHICS::GraphicsContext& context)
	{
		auto& currentActiveLightUpload = ActiveLightsUpload[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		auto& currentLightUpload = LightsUpload[D_RENDERER_DEVICE::GetCurrentResourceIndex()];

		UINT* data = (UINT*)currentActiveLightUpload.Map();
		LightData* lightUploadData = (LightData*)currentLightUpload.Map();

		bool done = false;

		for (size_t i = 0; !done && i < (MaxNumLight + sizeof(UINT) * 8 - 1) / sizeof(UINT) * 8; i++)
		{
			UINT activeFlags = 0;

			for (size_t bitIdx = 0; bitIdx < sizeof(UINT) * 8; bitIdx++)
			{
				auto lightIdx = i * sizeof(UINT) * 8 + bitIdx;
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
				}
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight)
				{
					indexInVec = lightIdx - MaxNumDirectionalLight;
					LightVec = &PointLights;
					transformVec = &PointLightTransforms;
					activeVec = &ActivePointLight;
				}
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight + MaxNumSpotLight)
				{
					indexInVec = lightIdx - MaxNumDirectionalLight - MaxNumPointLight;
					LightVec = &SpotLights;
					transformVec = &SpotLightTransforms;
					activeVec = &ActiveSpotLight;
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
		context.GetCommandList()->CopyBufferRegion(LightsBufferGpu.GetResource(), 0, currentLightUpload.GetResource(), 0, currentLightUpload.GetBufferSize());
		context.GetCommandList()->CopyBufferRegion(ActiveLightsBufferGpu.GetResource(), 0, currentActiveLightUpload.GetResource(), 0, currentActiveLightUpload.GetBufferSize());
		context.TransitionResource(LightsBufferGpu, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

		context.PIXEndEvent();
		context.TransitionResource(ActiveLightsBufferGpu, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);

	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetLightMaskHandle()
	{
		return ActiveLightsBufferGpu.GetSRV();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetLightDataHandle()
	{
		return LightsBufferGpu.GetSRV();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetShadowTextureArrayHandle()
	{
		return ShadowTextureArrayBuffer.GetSRV();
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

	void RenderDirectionalShadow(D_RENDERER::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData& light, int lightGloablIndex)
	{
		D_MATH_CAMERA::ShadowCamera cam;
		cam.UpdateMatrix(-Vector3(light.Direction), Vector3(0.f, 0.f, 0.f), Vector3(5000, 3000, 3000), ShadowBufferWidth, ShodowBufferHeight, ShadowBufferDepthPercision);
		light.ShadowMatrix = cam.GetShadowMatrix();
		GlobalConstants globals;
		globals.ViewProj = light.ShadowMatrix;

		auto& shadowBuffer = ShadowBuffers[lightGloablIndex];

		shadowBuffer.BeginRendering(context);

		sorter.SetCamera(cam);
		sorter.SetDepthStencilTarget(shadowBuffer);
		sorter.RenderMeshes(MeshSorter::kZPass, context, globals);

		context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		context.TransitionResource(ShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		context.CopySubresource(ShadowTextureArrayBuffer, lightGloablIndex, shadowBuffer, 0);

		context.TransitionResource(ShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void RenderSpotShadow(D_RENDERER::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, LightData& light, int lightGloablIndex)
	{
		D_MATH_CAMERA::Camera shadowCamera;
		shadowCamera.SetEyeAtUp(light.Position, Vector3(light.Position) + Vector3(light.Direction), Vector3(0, 1, 0));
		shadowCamera.SetPerspectiveMatrix(D_MATH::ACos(light.SpotAngles.y) * 2, 1.0f, light.Range * .001f, light.Range * 1.0f);
		shadowCamera.Update();
		light.ShadowMatrix = shadowCamera.GetViewProjMatrix();

		GlobalConstants globals;
		globals.ViewProj = light.ShadowMatrix;

		auto& shadowBuffer = ShadowBuffers[lightGloablIndex];

		shadowBuffer.BeginRendering(context);

		sorter.SetCamera(shadowCamera);
		sorter.SetDepthStencilTarget(shadowBuffer);
		sorter.RenderMeshes(MeshSorter::kZPass, context, globals);

		context.TransitionResource(shadowBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE);

		context.TransitionResource(ShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		context.CopySubresource(ShadowTextureArrayBuffer, lightGloablIndex, shadowBuffer, 0);

		context.TransitionResource(ShadowTextureArrayBuffer, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void RenderShadows(D_CONTAINERS::DVector<RenderItem> const& shadowRenderItems)
	{

		// TODO: Create sorter and input render item per light source
		MeshSorter sorter(MeshSorter::kShadows);
		for (auto const& ri : shadowRenderItems) sorter.AddMesh(ri, 0.1);

		auto& shadowContext = D_GRAPHICS::GraphicsContext::Begin();
		for (int i = 0; i < MaxNumLight; i++)
		{

			//D_JOB::AssignTask([&](int, int) {

			sorter.Reset();

			if (i < MaxNumDirectionalLight)
			{
				auto& light = DirectionalLights[i];
				if (!light.CastsShadow || !ActiveDirectionalLight[i].LightActive || !ActiveDirectionalLight[i].ComponentActive)
					continue;
				RenderDirectionalShadow(sorter, shadowContext, light, i);
			}
			else if (i >= MaxNumPointLight + MaxNumDirectionalLight)
			{
				auto idx = i - (MaxNumPointLight + MaxNumDirectionalLight);
				auto& light = SpotLights[idx];
				if (!light.CastsShadow || !ActiveSpotLight[idx].LightActive || !ActiveSpotLight[idx].ComponentActive)
					continue;
				RenderSpotShadow(sorter, shadowContext, light, i);
			}

			//});
		}
		shadowContext.Finish();

		if (D_JOB::IsMainThread())
			D_JOB::WaitForThreadsToFinish();
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

