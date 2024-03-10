#pragma once

#include "Renderer/Light/LightContext.hpp"

#include <Graphics/GraphicsUtils/Buffers/ShadowBuffer.hpp>
#include <Math/Camera/Camera.hpp>
#include <Math/Camera/ShadowCamera.hpp>

#ifndef D_RENDERER_RAST_LIGHT
#define D_RENDERER_RAST_LIGHT Darius::Renderer::Rasterization::Light
#endif // !D_RENDERER_RAST_LIGHT

namespace Darius::Renderer
{
	struct RenderItem;

	namespace Rasterization
	{
	
		class MeshSorter;

	}
}

namespace Darius::Renderer::Rasterization::Light
{
	class RasterizationShadowedLightContext : public D_RENDERER_LIGHT::LightContext
	{

	public:
		ALIGN_DECL_256 struct LightConfigBuffer
		{
			float mDirectionalShadowBufferTexelSize;
			float mPointShadowBufferTexelSize;
			float mSpotShadowBufferTexelSize;
		};

	public:

		RasterizationShadowedLightContext(UINT directionalShadowBufferDimansion = 4096u, UINT pointShadowBufferDimansion = 512u, UINT spotShadowBufferDimansion = 1024u, UINT shadowBufferDepthPercision = 16u);
		virtual ~RasterizationShadowedLightContext();

		virtual void							Update(D_MATH_CAMERA::Camera const& viewerCamera) override;

		void									GetShadowTextureArraysHandles(D3D12_CPU_DESCRIPTOR_HANDLE& directional, D3D12_CPU_DESCRIPTOR_HANDLE& point, D3D12_CPU_DESCRIPTOR_HANDLE& spot) const;

		void									RenderShadows(D_CONTAINERS::DVector<RenderItem> const& shadowRenderItems, D_GRAPHICS::GraphicsContext& shadowContext);

		void									GetLightConfigBufferData(LightConfigBuffer& outLightConfig) const;

	private:

		// Calc Shadow Camera Funcs
		void CalculateSpotShadowCamera(D_RENDERER_LIGHT::LightData& light, int spotLightIndex);
		void CalculateDirectionalShadowCamera(D_MATH_CAMERA::Camera const& viewerCamera, D_RENDERER_LIGHT::LightData& light, int directionalIndex);
		void CalculatePointShadowCamera(D_RENDERER_LIGHT::LightData& light, int pointLightIndex);

		// Render Light Shadow Funcs
		void RenderDirectionalShadow(Darius::Renderer::Rasterization::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, D_RENDERER_LIGHT::LightData const& light, int directionalIndex);
		void RenderSpotShadow(Darius::Renderer::Rasterization::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, D_RENDERER_LIGHT::LightData const& light, int spotLightIndex);
		void RenderPointShadow(Darius::Renderer::Rasterization::MeshSorter& sorter, D_GRAPHICS::GraphicsContext& context, D_RENDERER_LIGHT::LightData const& light, int pointLightIndex);

		// Config
		uint32_t							mDirectionalShadowBufferWidth;
		uint32_t							mPointShadowBufferWidth;
		uint32_t							mSpotShadowBufferWidth;
		uint32_t							mShadowBufferDepthPrecision;

		// SRV Shadow Buffers
		D_CONTAINERS::DVector<D_GRAPHICS_BUFFERS::ShadowBuffer> mShadowBuffersDirectional;
		D_CONTAINERS::DVector<D_GRAPHICS_BUFFERS::ShadowBuffer> mShadowBuffersSpot;
		D_CONTAINERS::DVector<D_GRAPHICS_BUFFERS::ShadowBuffer> mShadowBuffersPoint;

		// RTV Shadow Buffers
		D_GRAPHICS_BUFFERS::ColorBuffer		mDirectionalShadowTextureArrayBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer		mPointShadowTextureArrayBuffer;
		D_GRAPHICS_BUFFERS::ColorBuffer		mSpotShadowTextureArrayBuffer;

		// Render Cameras
		D_CONTAINERS::DVector<D_MATH_CAMERA::ShadowCamera>	mDirectionalShadowCameras;
		D_CONTAINERS::DVector<D_MATH_CAMERA::Camera>		mSpotShadowCameras;
		D_CONTAINERS::DVector<D_MATH_CAMERA::Camera>		mPointShadowCameras;

	};


	INLINE void RasterizationShadowedLightContext::GetShadowTextureArraysHandles(D3D12_CPU_DESCRIPTOR_HANDLE& directional, D3D12_CPU_DESCRIPTOR_HANDLE& point, D3D12_CPU_DESCRIPTOR_HANDLE& spot) const
	{
		directional = mDirectionalShadowTextureArrayBuffer.GetSRV();
		point = mPointShadowTextureArrayBuffer.GetSRV();
		spot = mSpotShadowTextureArrayBuffer.GetSRV();
	}

}