#pragma once

#include "CommandContext.hpp"

#include <Core/Containers/Vector.hpp>
#include <Math/VectorMath.hpp>

#ifndef D_LIGHT
#define D_LIGHT Darius::Renderer::LightManager
#endif // !D_LIGHT

using namespace D_CONTAINERS;
using namespace D_MATH;

namespace Darius::Renderer::LightManager
{
	constexpr int		MaxNumDirectionalLight = 6;
	constexpr int		MaxNumPointLight = 125;
	constexpr int		MaxNumSpotLight = 125;
	constexpr int		MaxNumLight = MaxNumDirectionalLight + MaxNumPointLight + MaxNumSpotLight;

	struct LightData
	{
		XMFLOAT4		Color = Vector4(kOne);
		XMFLOAT4		Direction = {0.f, 0.f, -1.f, 0.f };// Directional/Spot light only
		XMFLOAT4		Position = Vector4(kZero);  // Point light only
		float			FalloffStart = 1.f;			// Point/Spot light only
		float			FalloffEnd = 10.f;			// Point/Spot light only
		float			SpotPower = 64.f;			// Spot light only

	};

	void				Initialize();
	void				Shutdown();

	void				UpdateBuffers(D_GRAPHICS::GraphicsContext& context);
	D3D12_CPU_DESCRIPTOR_HANDLE GetLightMaskHandle();
}
