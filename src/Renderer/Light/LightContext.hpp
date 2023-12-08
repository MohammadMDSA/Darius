#pragma once

#include "LightCommon.hpp"

#include <Core/Containers/Vector.hpp>
#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Buffers/GpuBuffer.hpp>
#include <Math/Camera/Camera.hpp>

#ifndef D_RENDERER_LIGHT
#define D_RENDERER_LIGHT Darius::Renderer::Light
#endif // !D_RENDERER_LIGHT


namespace Darius::Renderer::Light
{
	class LightContext
	{
	public:

		LightContext();
		virtual ~LightContext();

		virtual void						Update(D_MATH_CAMERA::Camera const& viewerCamera) = 0;

		INLINE D3D12_CPU_DESCRIPTOR_HANDLE	GetLightsStatusBufferDescriptor() const { return mLightsStatusGpuBuffer.GetSRV(); }
		INLINE D3D12_CPU_DESCRIPTOR_HANDLE	GetLightsDataBufferDescriptor() const { return mLightsDataGpuBuffer.GetSRV(); }

		LightData& AddLightSource(LightData const& lightData, LightSourceType type);
		LightData& AddLightSource(LightData const& lightData, LightSourceType type, UINT& outIndex);

		UINT								GetNumberOfDirectionalLights() const;
		UINT								GetNumberOfPointLights() const;
		UINT								GetNumberOfSpotLights() const;

		LightData const&					GetDirectionalLightData(UINT index) const;
		LightData const&					GetPointLightData(UINT index) const;
		LightData const&					GetSpotLightData(UINT index) const;

		void								UpdateBuffers(D_GRAPHICS::CommandContext& context, D3D12_RESOURCE_STATES buffersReadyState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	
	protected:

		void								Reset();

	private:

		D_CONTAINERS::DVector<LightData>				mDirectionalLights;
		D_CONTAINERS::DVector<LightData>				mPointLights;
		D_CONTAINERS::DVector<LightData>				mSpotLights;

		D_GRAPHICS_BUFFERS::UploadBuffer				mLightsStatusUpload;
		D_GRAPHICS_BUFFERS::UploadBuffer				mLightsDataUpload;
		D_GRAPHICS_BUFFERS::ByteAddressBuffer			mLightsStatusGpuBuffer;
		D_GRAPHICS_BUFFERS::StructuredBuffer			mLightsDataGpuBuffer;

	};

	INLINE UINT LightContext::GetNumberOfDirectionalLights() const
	{
		return (UINT)mDirectionalLights.size();
	}

	INLINE UINT LightContext::GetNumberOfPointLights() const
	{
		return (UINT)mPointLights.size();
	}

	INLINE UINT LightContext::GetNumberOfSpotLights() const
	{
		return (UINT)mSpotLights.size();
	}

	INLINE LightData const& LightContext::GetDirectionalLightData(UINT index) const
	{
		D_ASSERT(GetNumberOfDirectionalLights() > index);
		return mDirectionalLights[index];
	}

	INLINE LightData const& LightContext::GetPointLightData(UINT index) const
	{
		D_ASSERT(GetNumberOfPointLights() > index);
		return mPointLights[index];
	}

	INLINE LightData const& LightContext::GetSpotLightData(UINT index) const
	{
		D_ASSERT(GetNumberOfSpotLights() > index);
		return mSpotLights[index];
	}


}