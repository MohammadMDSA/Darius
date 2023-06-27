#pragma once

#include "Renderer/FrameResource.hpp"

#include <Core/Containers/Vector.hpp>
#include <Graphics/CommandContext.hpp>
#include <Math/VectorMath.hpp>
#include <Math/Camera/Camera.hpp>

#include "LightManager.generated.hpp"

#ifndef D_LIGHT_RAST
#define D_LIGHT_RAST Darius::Renderer::Rasterization::LightManager
#endif // !D_LIGHT

namespace Darius::Renderer::Rasterization::LightManager
{
	constexpr UINT		MaxNumDirectionalLight = 6;
	constexpr UINT		MaxNumPointLight = 125;
	constexpr UINT		MaxNumSpotLight = 125;
	constexpr UINT		MaxNumLight = MaxNumDirectionalLight + MaxNumPointLight + MaxNumSpotLight;


	enum class DEnum(Serialize) LightSourceType
	{
		DirectionalLight,
		PointLight,
		SpotLight
	};


	struct DStruct(Serialize) LightData
	{
		DField(Serialize)
		DirectX::XMFLOAT3	Color = D_MATH::Vector3(D_MATH::kOne);

		DirectX::XMFLOAT3	Direction = { 0.f, 0.f, -1.f };				// Directional/Spot light only

		DirectX::XMFLOAT3	Position = D_MATH::Vector3(D_MATH::kZero);  // Point light only

		DField(Serialize)
		float				Intencity = 1.f;							// Point/Spot light only

		DField(Serialize)
		float				Range = 10.f;								// Point/Spot light only

		DirectX::XMFLOAT2	SpotAngles = { 1000.f, 0.8f };				// Spot light only
		DirectX::XMFLOAT4X4	ShadowMatrix;
		bool				CastsShadow = true;
		DirectX::XMFLOAT3	padding;
	};

	void				Initialize();
	void				Shutdown();
	void				Reset();
	void				Update();

	void				UpdateBuffers(D_GRAPHICS::GraphicsContext& context, D_MATH_CAMERA::Camera const* viewrCamera);
	void				RenderShadows(D_MATH_CAMERA::Camera const& viewerCamera, D_CONTAINERS::DVector<D_RENDERER_FRAME_RESOURCE::RenderItem> const& shadowRenderItems, D_GRAPHICS::GraphicsContext& shadowContext);

	D3D12_CPU_DESCRIPTOR_HANDLE GetLightMaskHandle();
	D3D12_CPU_DESCRIPTOR_HANDLE GetLightDataHandle();
	void GetShadowTextureArrayHandle(D3D12_CPU_DESCRIPTOR_HANDLE& directional, D3D12_CPU_DESCRIPTOR_HANDLE& point, D3D12_CPU_DESCRIPTOR_HANDLE& spot);

	D_H_SERIALIZE_ENUM(LightSourceType,
		{
			{ LightSourceType::DirectionalLight, "DirectionalLight" },
			{ LightSourceType::PointLight, "PointLight" },
			{ LightSourceType::SpotLight, "SpotLight" }
		});
}
