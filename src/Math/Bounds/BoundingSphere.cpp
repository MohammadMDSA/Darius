//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "Math/pch.hpp"
#include "BoundingSphere.hpp"

#include "BoundingBox.hpp"
#include "Math/Camera/Frustum.hpp"
#include "Math/Ray.hpp"


namespace Darius::Math::Bounds
{
    BoundingSphere BoundingSphere::Union(const BoundingSphere& rhs)
    {
        float radA = GetRadius();
        if (radA == 0.0f)
            return rhs;

        float radB = rhs.GetRadius();
        if (radB == 0.0f)
            return *this;

        Vector3 diff = GetCenter() - rhs.GetCenter();
        float dist = Length(diff);

        // Safe normalize vector between sphere centers
        diff = dist < 1e-6f ? Vector3(kXUnitVector) : diff * Recip(dist);

        Vector3 extremeA = GetCenter() + diff * Max(radA, radB - dist);
        Vector3 extremeB = rhs.GetCenter() - diff * Max(radB, radA - dist);

        return BoundingSphere((extremeA + extremeB) * 0.5f, Length(extremeA - extremeB) * 0.5f);
    }

    ContainmentType BoundingSphere::Contains(Vector3 const& point) const
    {
        return (ContainmentType)DirectX::BoundingSphere::Contains(point);
    }

    ContainmentType BoundingSphere::Contains(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2) const
    {
        return (ContainmentType)DirectX::BoundingSphere::Contains(v0, v1, v2);
    }

    ContainmentType BoundingSphere::Contains(BoundingSphere const& sphere) const
    {
        return (ContainmentType)DirectX::BoundingSphere::Contains(sphere);
    }

    ContainmentType BoundingSphere::Contains(AxisAlignedBox const& aabb) const
    {
        return (ContainmentType)DirectX::BoundingSphere::Contains((DirectX::BoundingBox const&)aabb);
    }

    ContainmentType BoundingSphere::Contains(OrientedBox const& orientedBox) const
    {
        return (ContainmentType)DirectX::BoundingSphere::Contains((DirectX::BoundingOrientedBox const&)orientedBox);
    }

    ContainmentType BoundingSphere::Contains(Darius::Math::Camera::Frustum const& frustum) const
    {
        if (!frustum.Intersects(*this))
            return ContainmentType::Disjoint;

        Vector3 center = GetCenter();
        float radius = GetRadius();
        float radiusSq = radius * radius;

        bool allInside = true;

        for (UINT i = 0u; i < Darius::Math::Camera::Frustum::CornerID::_kNumCorners; i++)
        {
            auto corner = frustum.GetFrustumCorner((Darius::Math::Camera::Frustum::CornerID)i);
            auto distSq = (center - corner).LengthSquare();
            allInside &= distSq <= radiusSq;

            if (allInside)
                return ContainmentType::Intersects;
        }

        return ContainmentType::Contains;
    }

    bool BoundingSphere::Intersects(BoundingSphere const& sphere) const
    {
        return DirectX::BoundingSphere::Intersects(sphere);
    }

    bool BoundingSphere::Intersects(AxisAlignedBox const& aabb) const
    {
        return DirectX::BoundingSphere::Intersects((DirectX::BoundingBox const&)aabb);
    }

    bool BoundingSphere::Intersects(OrientedBox const& orientedBox) const
    {
        return DirectX::BoundingSphere::Intersects((DirectX::BoundingOrientedBox const&)orientedBox);
    }

    bool BoundingSphere::Intersects(Darius::Math::Camera::Frustum const& frustum) const
    {
        return frustum.Intersects(*this);
    }

    bool BoundingSphere::Intersects(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2)
    {
        return DirectX::BoundingSphere::Intersects(v0, v1, v2);
    }

    bool BoundingSphere::Intersects(Darius::Math::Ray const& ray, _OUT_ float& dist) const
    {
        return DirectX::BoundingSphere::Intersects(ray.GetOrigin(), ray.GetDirection(), dist);
    }

    bool BoundingSphere::Intersects(BoundingPlane const& plane) const
    {
        return DirectX::BoundingSphere::Intersects(static_cast<Vector4>(plane));
    }

}