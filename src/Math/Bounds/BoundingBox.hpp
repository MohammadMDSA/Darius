#pragma once

#ifndef D_MATH_BOUNDS
#define D_MATH_BOUNDS Darius::Math::Bounds
#endif

#include "CollisionCommon.hpp"
#include "Math/VectorMath.hpp"
#include "Math/Transform.hpp"

namespace Darius::Math
{
	class Ray;

	namespace Camera
	{
		class Frustum;
	}
}

namespace Darius::Math::Bounds
{
	class BoundingSphere;
	class BoundingPlane;
	class OrientedBox;

	class AxisAlignedBox : private DirectX::BoundingBox
	{
	public:
		AxisAlignedBox() : DirectX::BoundingBox() {}
		AxisAlignedBox(EZeroTag) : DirectX::BoundingBox({ 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }) {}
		AxisAlignedBox(Vector3 min, Vector3 max) : DirectX::BoundingBox((min + max) / 2, (max - min) / 2) {}

		INLINE Vector3 GetMin() const { return Vector3(DirectX::BoundingBox::Center) - Vector3(DirectX::BoundingBox::Extents); }
		INLINE Vector3 GetMax() const { return Vector3(DirectX::BoundingBox::Center) + Vector3(DirectX::BoundingBox::Extents); }
		INLINE void SetMinMax(Vector3 min, Vector3 max)
		{
			DirectX::BoundingBox::Center = (DirectX::XMFLOAT3)((max + min) / 2);
			DirectX::BoundingBox::Extents = (DirectX::XMFLOAT3)((max - min) / 2);
		}

		INLINE void AddPoint(Vector3 point)
		{
			auto min = Min(point, GetMin());
			auto max = Max(point, GetMax());
			SetMinMax(min, max);
		}

		INLINE void AddBoundingBox(const AxisAlignedBox& box)
		{
			auto min = box.GetMin();
			auto max = box.GetMax();

			auto newMin = Min(min, GetMin());
			auto newMax = Max(max, GetMax());
			SetMinMax(newMin, newMax);
		}

		INLINE AxisAlignedBox Union(const AxisAlignedBox& box)
		{
			return AxisAlignedBox(Min(GetMin(), box.GetMin()), Max(GetMax(), box.GetMax()));
		}

		ContainmentType Contains(Vector3 const& point) const;
		// Triangle test
		ContainmentType Contains(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2) const;
		ContainmentType Contains(BoundingSphere const& sphere) const;
		ContainmentType Contains(AxisAlignedBox const& aabb) const;
		ContainmentType Contains(OrientedBox const& orientedBox) const;
		ContainmentType Contains(Darius::Math::Camera::Frustum const& frustum) const;

		bool Intersects(BoundingSphere const& sphere) const;
		bool Intersects(AxisAlignedBox const& aabb) const;
		bool Intersects(OrientedBox const& orientedBox) const;
		bool Intersects(Darius::Math::Camera::Frustum const& frustum) const;
		// Triangle-sphere test
		bool Intersects(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2);
		bool Intersects(Darius::Math::Ray const& ray, _OUT_ float& dist) const;
		bool Intersects(BoundingPlane const& plane) const;

		Vector3 GetCenter() const { return Center; }
		Vector3 GetDimensions() const { return GetExtents() * 2; }
		// Distance from center to each side;
		Vector3 GetExtents() const { return Extents; }

		// Index from 0 to 7 for 8 corners
		Vector3 GetCornerLocal(UINT index) const;
		Vector3 GetCorner(UINT index) const;
	};

	class OrientedBox : private DirectX::BoundingOrientedBox
	{
	public:
		OrientedBox() : DirectX::BoundingOrientedBox() {}

		OrientedBox(const AxisAlignedBox& box) : DirectX::BoundingOrientedBox(box.GetCenter(), box.GetExtents(), { 0.f, 0.f, 0.f, 1.f }) { }

		INLINE OrientedBox Transform(AffineTransform const& xform) const
		{
			OrientedBox result;
			DirectX::BoundingOrientedBox::Transform(result, xform);
			return result;
		}

		ContainmentType Contains(Vector3 const& point) const;
		// Triangle test
		ContainmentType Contains(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2) const;
		ContainmentType Contains(BoundingSphere const& sphere) const;
		ContainmentType Contains(AxisAlignedBox const& aabb) const;
		ContainmentType Contains(OrientedBox const& orientedBox) const;

		bool Intersects(BoundingSphere const& sphere) const;
		bool Intersects(AxisAlignedBox const& aabb) const;
		bool Intersects(OrientedBox const& orientedBox) const;
		// Triangle-sphere test
		bool Intersects(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2);
		bool Intersects(Darius::Math::Ray const& ray, _OUT_ float& dist) const;
		bool Intersects(BoundingPlane const& plane) const;

		INLINE Vector3 GetDimensions() const { return GetExtents() * 2; }
		INLINE Vector3 GetCenter() const { return DirectX::BoundingOrientedBox::Center; }
		INLINE Vector3 GetExtents() const { return DirectX::BoundingOrientedBox::Extents; }

	};

	INLINE OrientedBox operator* (AffineTransform const& xform, OrientedBox const& obb)
	{
		return obb.Transform(xform);
	}


	INLINE OrientedBox operator* (UniformTransform const& xform, OrientedBox const& obb)
	{
		return AffineTransform(xform) * obb;
	}

	INLINE OrientedBox operator* (UniformTransform const& xform, AxisAlignedBox const& aabb)
	{
		return AffineTransform(xform) * OrientedBox(aabb);
	}

	INLINE Vector3 AxisAlignedBox::GetCornerLocal(UINT index) const
	{
		D_ASSERT_M(index < 8, "Out of bounds index of the box corner");

		static const Vector3 boxOffset[8] =
		{
			{ -1.0f, -1.0f,  1.0f },
			{  1.0f, -1.0f,  1.0f },
			{  1.0f,  1.0f,  1.0f },
			{ -1.0f,  1.0f,  1.0f },
			{ -1.0f, -1.0f, -1.0f },
			{  1.0f, -1.0f, -1.0f },
			{  1.0f,  1.0f, -1.0f },
			{ -1.0f,  1.0f, -1.0f },
		};

		return GetExtents() * boxOffset[index];
	}

	INLINE Vector3 AxisAlignedBox::GetCorner(UINT index) const
	{
		return GetCornerLocal(index) + GetCenter();
	}

} // namespace Math
