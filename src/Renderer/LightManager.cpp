#include "pch.hpp"
#include "LightManager.hpp"

#include "FrameResource.hpp"
#include "GraphicsUtils/Buffers/GpuBuffer.hpp"
#include "GraphicsUtils/Buffers/UploadBuffer.hpp"
#include "RenderDeviceManager.hpp"

using namespace D_GRAPHICS_BUFFERS;

namespace Darius::Renderer::LightManager
{
	bool				_initialized = false;

	DVector<LightData>	DirectionalLights;
	DVector<LightData>	PointLights;
	DVector<LightData>	SpotLights;

	DVector<bool>		DirLightActive;
	DVector<bool>		PointLightActive;
	DVector<bool>		SpotLightActive;

	D_GRAPHICS_BUFFERS::UploadBuffer ActiveLightsUpload[D_RENDERER_FRAME_RESOUCE::gNumFrameResources];
	ByteAddressBuffer	ActiveLightsBufferGpu;

	void Initialize()
	{
		D_ASSERT(!_initialized);
		DirectionalLights.resize(MaxNumDirectionalLight);
		DirLightActive.resize(MaxNumDirectionalLight);
		PointLights.resize(MaxNumPointLight);
		PointLightActive.resize(MaxNumPointLight);
		SpotLights.resize(MaxNumSpotLight);
		SpotLightActive.resize(MaxNumSpotLight);

		for (size_t i = 0; i < MaxNumDirectionalLight; i++)
		{
			DirLightActive[i] = false;
		}

		for (size_t i = 0; i < MaxNumPointLight; i++)
		{
			PointLightActive[i] = false;
		}

		for (size_t i = 0; i < MaxNumSpotLight; i++)
		{
			SpotLightActive[i] = false;
		}
		PointLightActive[0] = true;

		// Initializing buffers
		size_t elemSize = sizeof(UINT) * 8;
		size_t count = (MaxNumLight + elemSize - 1) / elemSize;
		for (size_t i = 0; i < D_RENDERER_FRAME_RESOUCE::gNumFrameResources; i++)
		{
			ActiveLightsUpload[i].Create(L"Active Light Upload", count * sizeof(UINT));
		}
		ActiveLightsBufferGpu.Create(L"Active Light Buffer", count, sizeof(UINT), nullptr);

	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

	void UpdateBuffers(D_GRAPHICS::GraphicsContext& context)
	{
		auto& currentUpload = ActiveLightsUpload[D_RENDERER_DEVICE::GetCurrentResourceIndex()];
		UINT* data = (UINT*)currentUpload.Map();

		bool done = false;

		for (size_t i = 0; !done && i < (MaxNumLight + sizeof(UINT) * 8 - 1) / sizeof(UINT) * 8; i++)
		{
			UINT activeFlags = 0;

			for (size_t bitIdx = 0; bitIdx < sizeof(UINT) * 8; bitIdx++)
			{
				auto lightIdx = i * bitIdx * sizeof(UINT) + bitIdx;

				// Setting light status
				bool lightStatus = false;
				if (lightIdx < MaxNumDirectionalLight)
					lightStatus = DirLightActive[lightIdx];
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight)
					lightStatus = PointLightActive[lightIdx - MaxNumDirectionalLight];
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight + MaxNumSpotLight)
					lightStatus = SpotLightActive[lightIdx - MaxNumDirectionalLight - MaxNumPointLight];
				else
				{
					done = true;
					break;
				}

				if (lightStatus)
					activeFlags =+ 1;
				activeFlags <<= 1;
			}
			auto tt = sizeof(UINT);
			data[i] = activeFlags;
		}

		currentUpload.Unmap();

		// Uploading...
		context.TransitionResource(ActiveLightsBufferGpu, D3D12_RESOURCE_STATE_COPY_DEST, true);
		context.GetCommandList()->CopyBufferRegion(ActiveLightsBufferGpu.GetResource(), 0, currentUpload.GetResource(), 0, currentUpload.GetBufferSize());
		context.TransitionResource(ActiveLightsBufferGpu, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	}

	D3D12_CPU_DESCRIPTOR_HANDLE GetLightMaskHandle()
	{
		return ActiveLightsBufferGpu.GetSRV();
	}
}

