#include "pch.hpp"
#include "Ray.hpp"

#include "Bounds\BoundingSphere.hpp"
#include "Bounds\BoundingBox.hpp"
#include "Bounds\BoundingPlane.hpp"

namespace Darius::Math
{
	bool Ray::Intersects(Bounds::BoundingSphere const& sphere, float& dist) const
	{
		return sphere.Intersects(*this, dist);
	}
	
	bool Ray::Intersects(Bounds::AxisAlignedBox const& aabb, float& dist) const
	{
		return aabb.Intersects(*this, dist);
	}
	
	bool Ray::Intersects(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2, float& dist) const
	{
		return DirectX::TriangleTests::Intersects(mPosition, mDirection, v0, v1, v2, dist);
	}

	bool Ray::Intersects(Bounds::BoundingPlane const& plane, float& dist) const
	{
		Vector4 p = plane;

		auto nd = plane.Dot(mDirection);

		// Ray is in parallel with the plane
		if (D_MATH::Abs(nd) <= DirectX::g_RayEpsilon[0])
		{
			dist = 0.f;
			return false;
		}

		// t = -(dot(n,origin) + D) / dot(n,dir)
		float d = plane.DistanceFromPoint(mPosition);
		d /= nd;
		d = -d;
		if (d < 0)
		{
			dist = 0.f;
			return false;
		}
		else
		{
			dist = d;
			return true;
		}
	}
}