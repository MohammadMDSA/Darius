#include "Math\pch.hpp"
#include "BoundingBox.hpp"

#include "BoundingSphere.hpp"
#include "BoundingPlane.hpp"
#include "Math/Camera/Frustum.hpp"
#include "Math/Ray.hpp"

namespace Darius::Math::Bounds
{
    AxisAlignedBox AxisAlignedBox::CreateFromSphere(BoundingSphere const& sp)
    {
        Vector3 center = sp.GetCenter();
        Vector3 extents = Vector3(sp.GetRadius());
        return AxisAlignedBox(center - extents, center + extents);
    }

    AxisAlignedBox AxisAlignedBox::CreateFromCenterAndExtents(Vector3 const& center, Vector3 const& extents)
    {
        return AxisAlignedBox(center - extents, center + extents);
    }

    ContainmentType AxisAlignedBox::Contains(Vector3 const& point) const
    {
        return (ContainmentType)DirectX::BoundingBox::Contains(point);
    }

    ContainmentType AxisAlignedBox::Contains(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2) const
    {
        return (ContainmentType)DirectX::BoundingBox::Contains(v0, v1, v2);
    }

    ContainmentType AxisAlignedBox::Contains(BoundingSphere const& sphere) const
    {
        return (ContainmentType)DirectX::BoundingBox::Contains((DirectX::BoundingSphere const&)sphere);
    }

    ContainmentType AxisAlignedBox::Contains(AxisAlignedBox const& aabb) const
    {
        return (ContainmentType)DirectX::BoundingBox::Contains((DirectX::BoundingBox const&)aabb);
    }

    ContainmentType AxisAlignedBox::Contains(OrientedBox const& orientedBox) const
    {
        return (ContainmentType)DirectX::BoundingBox::Contains((DirectX::BoundingOrientedBox const&)orientedBox);
    }

    ContainmentType AxisAlignedBox::Contains(Darius::Math::Camera::Frustum const& frustum) const
    {
        using namespace DirectX;
        if (!frustum.Intersects(*this))
            return ContainmentType::Disjoint;

        auto center = GetCenter();
        auto extents = GetExtents();

        auto allInside = XMVectorTrueInt();
        for (UINT i = 0; i < D_MATH_CAMERA::Frustum::_kNumCorners; i++)
        {
            auto corner = frustum.GetFrustumCorner((D_MATH_CAMERA::Frustum::CornerID)i);
            DirectX::XMVECTOR d = XMVectorAbs(XMVectorSubtract(corner, center));
            allInside = XMVectorAndInt(allInside, XMVectorLessOrEqual(d, extents));
        }

        return (XMVector3EqualInt(allInside, XMVectorTrueInt())) ? ContainmentType::Contains : ContainmentType::Intersects;

    }

    bool AxisAlignedBox::Intersects(BoundingSphere const& sphere) const
    {
        return DirectX::BoundingBox::Intersects((DirectX::BoundingSphere const&)sphere);
    }

    bool AxisAlignedBox::Intersects(AxisAlignedBox const& aabb) const
    {
        return DirectX::BoundingBox::Intersects(aabb);
    }

    bool AxisAlignedBox::Intersects(OrientedBox const& orientedBox) const
    {
        return DirectX::BoundingBox::Intersects((DirectX::BoundingOrientedBox const&)orientedBox);
    }

    bool AxisAlignedBox::Intersects(Darius::Math::Camera::Frustum const& frustum) const
    {
        return frustum.Intersects(*this);
    }

    // Triangle-sphere test
    bool AxisAlignedBox::Intersects(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2)
    {
        return DirectX::BoundingBox::Intersects(v0, v1, v2);
    }

    bool AxisAlignedBox::Intersects(Darius::Math::Ray const& ray, _OUT_ float& dist) const
    {
        return DirectX::BoundingBox::Intersects(ray.GetOrigin(), ray.GetDirection(), dist);
    }

    bool AxisAlignedBox::Intersects(BoundingPlane const& plane) const
    {
        return DirectX::BoundingBox::Intersects(Vector4(plane));
    }


    ContainmentType OrientedBox::Contains(Vector3 const& point) const
    {
        return (ContainmentType)DirectX::BoundingOrientedBox::Contains(point);
    }

    ContainmentType OrientedBox::Contains(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2) const
    {
        return (ContainmentType)DirectX::BoundingOrientedBox::Contains(v0, v1, v2);
    }

    ContainmentType OrientedBox::Contains(BoundingSphere const& sphere) const
    {
        return (ContainmentType)DirectX::BoundingOrientedBox::Contains((DirectX::BoundingSphere const&)sphere);
    }

    ContainmentType OrientedBox::Contains(AxisAlignedBox const& aabb) const
    {
        return (ContainmentType)DirectX::BoundingOrientedBox::Contains((DirectX::BoundingBox const&)aabb);
    }

    ContainmentType OrientedBox::Contains(OrientedBox const& orientedBox) const
    {
        return (ContainmentType)DirectX::BoundingOrientedBox::Contains(*this);
    }

    bool OrientedBox::Intersects(BoundingSphere const& sphere) const
    {
        return DirectX::BoundingOrientedBox::Intersects((DirectX::BoundingSphere const&)sphere);
    }

    bool OrientedBox::Intersects(AxisAlignedBox const& aabb) const
    {
        return DirectX::BoundingOrientedBox::Intersects((DirectX::BoundingBox const&)aabb);
    }

    bool OrientedBox::Intersects(OrientedBox const& orientedBox) const
    {
        return DirectX::BoundingOrientedBox::Intersects(*this);
    }

    bool OrientedBox::Intersects(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2)
    {
        return DirectX::BoundingOrientedBox::Intersects(v0, v1, v2);
    }

    bool OrientedBox::Intersects(Darius::Math::Ray const& ray, _OUT_ float& dist) const
    {
        return DirectX::BoundingOrientedBox::Intersects(ray.GetOrigin(), ray.GetDirection(), dist);
    }

    bool OrientedBox::Intersects(BoundingPlane const& plane) const
    {
        return DirectX::BoundingOrientedBox::Intersects(Vector4(plane));
    }

}
