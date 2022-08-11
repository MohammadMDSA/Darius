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
		Vector3			Color = Vector3(kZero);
		Vector3			Direction = Vector3::Down();// Directional/Spot light only
		Vector3			Position = Vector3(kZero);  // Point light only
		float			FalloffStart = 1.f;			// Point/Spot light only
		float			FalloffEnd = 10.f;			// Point/Spot light only
		float			SpotPower = 64.f;			// Spot light only

	};

}
