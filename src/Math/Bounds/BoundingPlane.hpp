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

#pragma once

#ifndef D_MATH_BOUNDS
#define D_MATH_BOUNDS Darius::Math::Bounds
#endif

#include "Math/VectorMath.hpp"

namespace Darius::Math::Bounds
{
    class BoundingPlane
    {
    public:

        BoundingPlane() {}
        BoundingPlane(Vector3 const& normalToPlane, float distanceFromOrigin) : m_repr(normalToPlane, distanceFromOrigin) {}
        BoundingPlane(Vector3 const& pointOnPlane, Vector3 const& normalToPlane);
        BoundingPlane(float A, float B, float C, float D) : m_repr(A, B, C, D) {}
        BoundingPlane(const BoundingPlane& plane) : m_repr(plane.m_repr) {}
        explicit BoundingPlane(Vector4 const& plane) : m_repr(plane) {}

        BoundingPlane& operator=(BoundingPlane const& other) = default;

        INLINE operator Vector4() const { return m_repr; }

        // Returns the direction the plane is facing.  (Warning:  might not be normalized.)
        INLINE Vector3 GetNormal(void) const { return Vector3(DirectX::XMVECTOR(m_repr)); }

        // Returns the vector4 representation of the plane.
        INLINE Vector4 GetVector4() const { return m_repr; }

        // Returns the point on the plane closest to the origin
        INLINE Vector3 GetPointOnPlane(void) const { return -GetNormal() * m_repr.GetW(); }

        // Distance from 3D point
        INLINE Scalar DistanceFromPoint(Vector3 const& point) const
        {
            return Math::Dot(point, GetNormal()) + m_repr.GetW();
        }

        // Distance from homogeneous point
        INLINE Scalar DistanceFromPoint(Vector4 const& point) const
        {
            return Math::Dot(point, m_repr);
        }

        Scalar IsPointOver(Vector3 const& point) const
        {
            return DistanceFromPoint(point) > 0.f;
        }

        INLINE float Dot(Vector3 const& vec) const
        {
            return DirectX::XMVectorGetX(DirectX::XMPlaneDotNormal(m_repr, vec));
        }

        // Most efficient way to transform a plane.  (Involves one quaternion-vector rotation and one dot product.)
        friend BoundingPlane operator* (const OrthogonalTransform& xform, BoundingPlane plane)
        {
            Vector3 normalToPlane = xform.GetRotation() * plane.GetNormal();
            float distanceFromOrigin = plane.m_repr.GetW() - Math::Dot(normalToPlane, xform.GetTranslation());
            return BoundingPlane(normalToPlane, distanceFromOrigin);
        }

        // Less efficient way to transform a plane (but handles affine transformations.)
        friend BoundingPlane operator* (const Matrix4& mat, BoundingPlane plane)
        {
            return BoundingPlane(Transpose(Invert(mat)) * plane.m_repr);
        }

    private:

        Vector4 m_repr;
    };

    using Plane = BoundingPlane;

    //=======================================================================================================
    // Inline implementations
    //
    inline BoundingPlane::BoundingPlane(Vector3 const& pointOnPlane, Vector3 const& normalToPlane)
    {
        // Guarantee a normal.  This constructor isn't meant to be called frequently, but if it is, we can change this.
        Vector3 normal = Normalize(normalToPlane);
        m_repr = Vector4(normal, -Math::Dot(pointOnPlane, normal));
    }

    //=======================================================================================================
    // Functions operating on planes
    //
    inline BoundingPlane PlaneFromPointsCCW(Vector3 A, Vector3 B, Vector3 C)
    {
        return BoundingPlane(A, Cross(B - A, C - A));
    }


} // namespace Math
