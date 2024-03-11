
#include "Renderer/pch.hpp"
#include "LightContext.hpp"

#include "Graphics/GraphicsDeviceManager.hpp"
#include "Graphics/GraphicsUtils/Profiling/Profiling.hpp"

#include "LightCommon.sgenerated.hpp"

using namespace D_CONTAINERS;

namespace Darius::Renderer::Light
{
	LightContext::LightContext()
	{
		mDirectionalLights.reserve(MaxNumDirectionalLight);
		mPointLights.reserve(MaxNumPointLight);
		mSpotLights.reserve(MaxNumSpotLight);

		CreateBuffers();
	}

	void LightContext::CreateBuffers()
	{
		// Calculating how many bytes we require to store MaxNumLight number of light status
		UINT elemSize = sizeof(UINT) * 8;
		UINT count = (MaxNumLight + elemSize - 1) / elemSize; // Ceil MaxNum / elemSize

		mLightsStatusUpload.Create(L"Active Light Upload", count * sizeof(UINT), D_GRAPHICS_DEVICE::gNumFrameResources);
		mLightsDataUpload.Create(L"Lights Data Upload", MaxNumLight * sizeof(LightData), D_GRAPHICS_DEVICE::gNumFrameResources);
		mLightsStatusGpuBuffer.Create(L"Active Light Gpu Buffer", (UINT)count, sizeof(UINT), nullptr);
		mLightsDataGpuBuffer.Create(L"Lights Data Gpu Buffer", MaxNumLight, sizeof(LightData));
	}

	LightContext::~LightContext()
	{
		DestroyBuffers();
	}

	void LightContext::DestroyBuffers()
	{
		mLightsStatusUpload.Destroy();
		mLightsStatusGpuBuffer.Destroy();
		mLightsDataUpload.Destroy();
		mLightsDataGpuBuffer.Destroy();
	}

	void LightContext::Reset()
	{
		mDirectionalLights.clear();
		mPointLights.clear();
		mSpotLights.clear();
	}

	void LightContext::UpdateBuffers(D_GRAPHICS::CommandContext& context, D3D12_RESOURCE_STATES buffersReadyState)
	{
		D_PROFILING::ScopedTimer _prof(L"Updating Light Buffers");

		auto frameResourceIndex = D_GRAPHICS_DEVICE::GetCurrentFrameResourceIndex();

		// Upload buffers state
		UINT* data = (UINT*)mLightsStatusUpload.MapInstance(frameResourceIndex);
		LightData* lightUploadData = (LightData*)mLightsDataUpload.MapInstance(frameResourceIndex);
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
					LightVec = &mDirectionalLights;
					lightSource = LightSourceType::DirectionalLight;
				}
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight)
				{
					indexInVec = lightIdx - MaxNumDirectionalLight;
					LightVec = &mPointLights;
					lightSource = LightSourceType::PointLight;
				}
				else if (lightIdx < MaxNumDirectionalLight + MaxNumPointLight + MaxNumSpotLight)
				{
					indexInVec = lightIdx - MaxNumDirectionalLight - MaxNumPointLight;
					LightVec = &mSpotLights;
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

					activeFlags += 1;

					lightUploadData[lightIdx] = lightData;
				}
				if (bitIdx < 31)
					activeFlags <<= 1;
			}
			data[i] = activeFlags;
		}

		mLightsStatusUpload.Unmap();
		mLightsDataUpload.Unmap();

		context.TransitionResource(mLightsStatusGpuBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
		context.TransitionResource(mLightsDataGpuBuffer, D3D12_RESOURCE_STATE_COPY_DEST);

		context.CopyBufferRegion(mLightsDataGpuBuffer, 0, mLightsDataUpload, frameResourceIndex * mLightsDataUpload.GetBufferSize(), mLightsDataUpload.GetBufferSize());
		context.CopyBufferRegion(mLightsStatusGpuBuffer, 0, mLightsStatusUpload, frameResourceIndex * mLightsStatusUpload.GetBufferSize(), mLightsStatusUpload.GetBufferSize());

		context.TransitionResource(mLightsStatusGpuBuffer, buffersReadyState);
		context.TransitionResource(mLightsDataGpuBuffer, buffersReadyState, true);
	}

	LightData& LightContext::AddLightSource(LightData const& lightData, LightSourceType type)
	{
		UINT index;
		return AddLightSource(lightData, type, index);
	}

	LightData& LightContext::AddLightSource(LightData const& lightData, LightSourceType type, UINT& outIndex)
	{
		switch (type)
		{
		case Darius::Renderer::Light::LightSourceType::DirectionalLight:
			outIndex = (UINT)mDirectionalLights.size();
			mDirectionalLights.push_back(lightData);
			return mDirectionalLights[outIndex];

		case Darius::Renderer::Light::LightSourceType::PointLight:
			outIndex = (UINT)mPointLights.size();
			mPointLights.push_back(lightData);
			return mPointLights[outIndex];

		case Darius::Renderer::Light::LightSourceType::SpotLight:
			outIndex = (UINT)mSpotLights.size();
			mSpotLights.push_back(lightData);
			return mSpotLights[outIndex];

		default:
			D_ASSERT_M(false, "Source type is not implemented");
			throw std::exception("Source type is not implemented");
		}
	}
}
