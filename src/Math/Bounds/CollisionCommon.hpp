#pragma once

#ifndef D_MATH_BOUNDS
#define D_MATH_BOUNDS Darius::Math::Bounds
#endif

namespace Darius::Math::Bounds
{
	enum class ContainmentType : uint8_t
	{
		Disjoint = 0,
		Intersects = 1,
		Contains = 2
	};

	enum class PlaneIntersectionType : uint8_t
	{
		Front = 0,
		Intersecting = 1,
		Back = 2
	};
}