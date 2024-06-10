#pragma once

#ifndef D_MATH_BOUNDS
#define D_MATH_BOUNDS Darius::Math::Bounds
#endif

#include "Math/VectorMath.hpp"

#include "CollisionCommon.hpp"

namespace Darius::Math
{
    class Ray;
}

namespace Darius::Math::Camera
{
    class Frustum;
}

namespace Darius::Math::Bounds
{
    class AxisAlignedBox;
    class OrientedBox;
    class BoundingPlane;

    class BoundingSphere : private DirectX::BoundingSphere
    {
    public:
        BoundingSphere() : DirectX::BoundingSphere() {}
        BoundingSphere(float x, float y, float z, float r) : DirectX::BoundingSphere({ x, y, z }, r) {}
        BoundingSphere(Vector3 center, Scalar radius) : DirectX::BoundingSphere(center, radius) {}
        BoundingSphere(EZeroTag) : DirectX::BoundingSphere() {}
        explicit BoundingSphere(const DirectX::XMFLOAT4& f4) : BoundingSphere(f4.x, f4.y, f4.z, f4.w) {}
        explicit BoundingSphere(Vector4 sphere) : DirectX::BoundingSphere(Vector3(sphere), sphere.GetW()) {}
        explicit operator Vector4() const { return Vector4(Center, Radius); }

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

        INLINE Vector3 GetCenter(void) const { return Center; }
        INLINE float GetRadius(void) const { return Radius; }

        BoundingSphere Union(const BoundingSphere& rhs);

        void AddPoint(Vector3 const& point);

        static BoundingSphere CreateFromPointList(Vector3 const* points, uint32_t count);
    };

    //=======================================================================================================
    // Inline implementations
    //

    INLINE BoundingSphere operator*(const OrthogonalTransform& trans, const BoundingSphere& sphere)
    {
        return BoundingSphere(trans * sphere.GetCenter(), sphere.GetRadius());
    }

    INLINE BoundingSphere operator*(const ScaleAndTranslation& trans, const BoundingSphere& sphere)
    {
        Vector4 scaleSphere = static_cast<Vector4>(sphere) * trans.GetScale();
        Vector4 translation = Vector4(trans.GetTranslation(), 0.f);
        return BoundingSphere(scaleSphere + translation);
    }

    INLINE BoundingSphere operator*(const UniformTransform& trans, const BoundingSphere& sphere)
    {
        return BoundingSphere(trans * sphere.GetCenter(), trans.GetScale() * sphere.GetRadius());
    }

    INLINE BoundingSphere operator*(const AffineTransform& trans, const BoundingSphere& sphere)
    {
        auto scale = trans.GetScale();
        
        return BoundingSphere(trans * sphere.GetCenter(), Max(scale.GetX(), Max(scale.GetY(), scale.GetZ())) * sphere.GetRadius());
    }

    INLINE BoundingSphere operator*(const Transform& trans, const BoundingSphere& sphere)
    {
        auto scale = trans.Scale;
        return BoundingSphere(trans * sphere.GetCenter(), Max(scale.GetX(), Max(scale.GetY(), scale.GetZ())) * sphere.GetRadius());
    }

}