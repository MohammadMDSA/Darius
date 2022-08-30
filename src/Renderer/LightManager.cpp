#include "pch.hpp"
#include "LightManager.hpp"

#include "FrameResource.hpp"
#include "GraphicsUtils/Buffers/GpuBuffer.hpp"
#include "GraphicsUtils/Buffers/UploadBuffer.hpp"
#include "RenderDeviceManager.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/Exceptions/Exception.hpp>

using namespace D_GRAPHICS_BUFFERS;
using namespace D_CONTAINERS;
using namespace D_MATH;

namespace Darius::Renderer::LightManager
{
	bool					_initialized = false;

	DVector<LightData>		DirectionalLights;
	DVector<LightData>		PointLights;
	DVector<LightData>		SpotLights;

	DVector<std::pair<bool, bool>>	ActiveDirectionalLight;
	DVector<std::pair<bool, bool>>	ActivePointLight;
	DVector<std::pair<bool, bool>>	ActiveSpotLight;

	DVector<Transform const*>		DirectionalLightTransforms;
	DVector<Transform const*>		PointLightTransforms;
	DVector<Transform const*>		SpotLightTransforms;

	// Gpu buffers
	D_GRAPHICS_BUFFERS::UploadBuffer	ActiveLightsUpload[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
	D_GRAPHICS_BUFFERS::UploadBuffer	LightsUpload[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
	ByteAddressBuffer					ActiveLightsBufferGpu;
	StructuredBuffer					LightsBufferGpu;

	INLINE DVector<std::pair<bool, bool>>*		GetAssociatedActiveLightWithType(LightSourceType type);
	INLINE DVector<LightData>*			GetAssociatedLightsWithType(LightSourceType type);
	INLINE DVector<Transform const*>*		GetAssociatedLightTransformsWithType(LightSourceType type);

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
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			ActiveLightsUpload[i].Create(L"Active Light Upload", count * sizeof(UINT));
			LightsUpload[i].Create(L"Light Upload", MaxNumLight * sizeof(LightData));
		}
		ActiveLightsBufferGpu.Create(L"Active Light Buffer", count, sizeof(UINT), nullptr);
		LightsBufferGpu.Create(L"Light Buffer", MaxNumLight, sizeof(LightData));

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
			DirectionalLightTransforms[i] = nullptr;
		}

		for (size_t i = 0; i < MaxNumPointLight; i++)
		{
			ActivePointLight[i] = { false, false };
			PointLightTransforms[i] = nullptr;
		}

		for (size_t i = 0; i < MaxNumSpotLight; i++)
		{
			ActiveSpotLight[i] = { false, false };
			SpotLightTransforms[i] = nullptr;
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
				auto lightIdx = i * bitIdx * sizeof(UINT) + bitIdx;
				DVector<LightData>* LightVec = nullptr;
				DVector<Transform const*>* transformVec = nullptr;
				DVector<std::pair<bool, bool>>* activeVec = nullptr;
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

				lightStatus = (*activeVec)[indexInVec].first && (*activeVec)[indexInVec].second;

				if (lightStatus)
				{
					// Setting location and direction
					auto trans = (*transformVec)[indexInVec];
					LightData& lightData = (*LightVec)[indexInVec];
					lightData.Position = Vector4(trans->Translation, 0.f);
					XMStoreFloat4(&lightData.Direction, XMVector3Rotate({ 0.f, 0.f, -1.f }, trans->Rotation));
					activeFlags = +1;

					lightUploadData[lightIdx] = lightData;
				}
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

	INLINE DVector<std::pair<bool, bool>>* GetAssociatedActiveLightWithType(LightSourceType type)
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
	}

	INLINE DVector<Transform const*>* GetAssociatedLightTransformsWithType(LightSourceType type)
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
	}

	int AccuireLightSource(LightSourceType type)
	{
		auto typeOwners = GetAssociatedActiveLightWithType(type);

		// Finding an index in light sources array that is no one is using
		int emptyIndex = -1;
		for (int i = 0; i < typeOwners->size(); i++)
		{
			if (!typeOwners->at(i).first)
			{
				emptyIndex = i;
				break;
			}
		}

		// No light source of specified type is available
		if (emptyIndex == -1)
			return -1;

		(*typeOwners)[emptyIndex].first = true;

		return emptyIndex;
	}

	int SwapLightSource(LightSourceType type, LightSourceType preType, int preIndex)
	{

		auto preTypeLights = GetAssociatedLightsWithType(preType);

		// Requesting the same type, so return what requester already own
		if (preType == type)
			preIndex;

		// Removing owner data to accuire new light
		ReleaseLight(preType, preIndex);

		auto newLight = AccuireLightSource(type);

		return newLight;
	}

	void ReleaseLight(LightSourceType preType, int preIndex)
	{
		(*GetAssociatedActiveLightWithType(preType))[preIndex].first = false;
	}

	void UpdateLight(LightSourceType type, int index, Transform const* trans, bool active, LightData const& light)
	{
		GetAssociatedActiveLightWithType(type)->at(index).second = active;
		if (!active)
			return;

		GetAssociatedLightTransformsWithType(type)->at(index) = trans;
		GetAssociatedLightsWithType(type)->at(index) = light;
	}

}

