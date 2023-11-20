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
        auto radius = GetRadius();
        auto radiusSq = radius * radius;
        auto distSq = (point - GetCenter()).LengthSquare();

        return distSq <= radiusSq ? ContainmentType::Contains : ContainmentType::Disjoint;
    }
    
    ContainmentType BoundingSphere::Contains(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2) const
    {
        auto center = GetCenter();
        auto radius = GetRadius();
        auto radiusSq = radius * radius;

        auto distSq = (v0 - center).LengthSquare();
        auto inside = distSq <= radiusSq;

        distSq = (v1 - center).LengthSquare();
        inside = inside && (distSq <= radiusSq);

        distSq = (v2 - center).LengthSquare();
        inside = inside && (distSq <= radiusSq);

        return inside ? ContainmentType::Contains : ContainmentType::Intersects;
    }

    ContainmentType BoundingSphere::Contains(BoundingSphere const& sphere) const
    {
        Vector3 center1 = GetCenter();
        float r1 = GetRadius();

        Vector3 center2 = sphere.GetCenter();
        float r2 = sphere.GetRadius();
        
        float dist = (center2 - center1).Length();
        
        return (r1 + r2 >= dist) ? ((r1 - r2 >= dist) ? ContainmentType::Contains : ContainmentType::Intersects) : ContainmentType::Disjoint;
    }

    ContainmentType BoundingSphere::Contains(AxisAlignedBox const& aabb) const
    {

    }

    ContainmentType BoundingSphere::Contains(OrientedBox const& orientedBox) const
    {

    }

    ContainmentType BoundingSphere::Contains(Darius::Math::Camera::Frustum const& frustum) const
    {

    }

    bool BoundingSphere::Intersects(BoundingSphere const& sphere) const
    {

    }

    bool BoundingSphere::Intersects(AxisAlignedBox const& aabb) const
    {

    }

    bool BoundingSphere::Intersects(OrientedBox const& orientedBox) const
    {

    }

    bool BoundingSphere::Intersects(Darius::Math::Camera::Frustum const& frustum) const
    {

    }

    bool BoundingSphere::Intersects(Vector3 const& v0, Vector3 const& v1, Vector3 const& v2)
    {

    }

    bool BoundingSphere::Intersects(Darius::Math::Plane const& plane) const
    {

    }

    bool BoundingSphere::Intersects(Darius::Math::Ray const& ray, _OUT_ float& dist) const
    {

    }

}