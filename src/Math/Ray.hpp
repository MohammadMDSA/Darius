#pragma once

#ifndef D_MATH
#define D_MATH Darius::Math
#endif

#include "Vector.hpp"

namespace Darius::Math
{
	namespace Bounds
	{
		class AxisAlignedBox;
		class OrientedBox;
		class BoundingSphere;
		class BoundingPlane;
	}

	class Ray
	{
	public:
		Ray() : mPosition(kZero), mDirection(Vector3::Forward) {}
		Ray(Vector3 const& pos, Vector3 const& dir) : mPosition(pos), mDirection(dir.Normalize()) {}

		Ray(Ray const&) = default;
		Ray& operator=(Ray const&) = default;
		
		Ray(Ray&&) = default;
		Ray& operator=(Ray&&) = default;

		// Comparision operators
		bool operator== (Ray const& other) const;
		bool operator!= (Ray const& other) const;

		INLINE Vector3 const& GetOrigin() const { return mPosition; }
		INLINE Vector3 const& GetDirection() const { return mDirection; }
		
		// Ray operators
		bool Intersects(Bounds::BoundingSphere const& sphere, _OUT_ float& dist) const;
		bool Intersects(Bounds::AxisAlignedBox const& aabb, _OUT_ float& dist) const;
		// Triangle test
		bool Intersects(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2, _OUT_ float& dist) const;
		bool Intersects(Bounds::BoundingPlane const& plane, _OUT_ float& dist) const;

	private:
		Vector3 mPosition;
		Vector3 mDirection;
	};
}