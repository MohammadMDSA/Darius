#pragma once


#include <Math/VectorMath.hpp>
#include <Utils/Common.hpp>

#include "LightCommon.generated.hpp"

#ifndef D_RENDERER_LIGHT
#define D_RENDERER_LIGHT Darius::Renderer::Light
#endif // !D_RENDERER_LIGHT


namespace Darius::Renderer::Light
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
		bool				CastsShadow = true;
		DirectX::XMFLOAT3	padding;
	};

	//D_STATIC_ASSERT(sizeof(LightData) == 52)
}
