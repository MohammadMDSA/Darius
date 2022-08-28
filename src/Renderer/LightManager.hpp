#pragma once

#include <Math/VectorMath.hpp>

#ifndef D_LIGHT
#define D_LIGHT Darius::Renderer::LightManager
#endif // !D_LIGHT

using namespace D_MATH;

namespace Darius::Renderer::LightManager
{

	struct LightData
	{
		XMFLOAT4		Color = Vector4(kZero);
		XMFLOAT4		Direction = {0.f, 0.f, -1.f, 0.f };// Directional/Spot light only
		XMFLOAT4		Position = Vector4(kZero);  // Point light only
		float			FalloffStart = 1.f;			// Point/Spot light only
		float			FalloffEnd = 10.f;			// Point/Spot light only
		float			SpotPower = 64.f;			// Spot light only

	};

}
