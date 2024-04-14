#pragma once

#define D_MATH_CAMERA Darius::Math::Camera

#include "Math/Bounds/BoundingPlane.hpp"
#include "Math/Bounds/BoundingSphere.hpp"
#include "Math/Bounds/BoundingBox.hpp"

namespace Darius::Math::Camera
{
    class Frustum
    {
    public:
        Frustum() {}

        Frustum(const Matrix4& ProjectionMatrix);

        enum CornerID
        {
            kNearLowerLeft, kNearUpperLeft, kNearLowerRight, kNearUpperRight,
            kFarLowerLeft, kFarUpperLeft, kFarLowerRight, kFarUpperRight, _kNumCorners
        };

        enum PlaneID
        {
            kNearPlane, kFarPlane, kLeftPlane, kRightPlane, kTopPlane, kBottomPlane
        };

        D_MATH_BOUNDS::AxisAlignedBox   GetAABB(float backwardsDepthBias = 1000.f) const;

        Vector3 const&                  GetFrustumCorner(CornerID id) const { return m_FrustumCorners[id]; }
        D_MATH_BOUNDS::BoundingPlane const& GetFrustumPlane(PlaneID id) const { return m_FrustumPlanes[id]; }

        // Test whether the bounding sphere intersects the frustum.  Intersection is defined as either being
        // fully contained in the frustum, or by intersecting one or more of the planes.
        bool                            Intersects(D_MATH_BOUNDS::BoundingSphere const& sphere) const;


        bool                            Intersects(D_MATH_BOUNDS::AxisAlignedBox const& aabb) const;

        Vector3 const*                  _GetCorners() const { return m_FrustumCorners; }
        D_MATH_BOUNDS::BoundingPlane const* _GetPlanes() const { return m_FrustumPlanes; }

        friend Frustum  operator* (const OrthogonalTransform& xform, const Frustum& frustum);	// Fast
        friend Frustum  operator* (const AffineTransform& xform, const Frustum& frustum);		// Slow
        friend Frustum  operator* (const Matrix4& xform, const Frustum& frustum);				// Slowest (and most general)

    private:

        // Perspective frustum constructor (for pyramid-shaped frusta)
        void ConstructPerspectiveFrustum(float HTan, float VTan, float NearClip, float FarClip);

        // Orthographic frustum constructor (for box-shaped frusta)
        void ConstructOrthographicFrustum(float Left, float Right, float Top, float Bottom, float NearClip, float FarClip);

        Vector3                         m_FrustumCorners[8];		// the corners of the frustum
        D_MATH_BOUNDS::BoundingPlane    m_FrustumPlanes[6];			// the bounding planes
    };

    //=======================================================================================================
    // Inline implementations
    //

    inline bool Frustum::Intersects(D_MATH_BOUNDS::BoundingSphere const& sphere) const
    {
        float radius = sphere.GetRadius();
        for (int i = 0; i < 6; ++i)
        {
            if (m_FrustumPlanes[i].DistanceFromPoint(sphere.GetCenter()) + radius < 0.0f)
                return false;
        }
        return true;
    }

    inline bool Frustum::Intersects(D_MATH_BOUNDS::AxisAlignedBox const& aabb) const
    {
        for (int i = 0; i < 6; ++i)
        {
            D_MATH_BOUNDS::BoundingPlane const& p = m_FrustumPlanes[i];
            Vector3 farCorner = Select(aabb.GetMin(), aabb.GetMax(), p.GetNormal() > Vector3(kZero));
            if (p.DistanceFromPoint(farCorner) < 0.0f)
                return false;
        }

        return true;
    }

    inline Frustum operator* (const OrthogonalTransform& xform, const Frustum& frustum)
    {
        Frustum result;

        for (int i = 0; i < 8; ++i)
            result.m_FrustumCorners[i] = xform * frustum.m_FrustumCorners[i];

        for (int i = 0; i < 6; ++i)
            result.m_FrustumPlanes[i] = xform * frustum.m_FrustumPlanes[i];

        return result;
    }

    inline Frustum operator* (const AffineTransform& xform, const Frustum& frustum)
    {
        Frustum result;

        for (int i = 0; i < 8; ++i)
            result.m_FrustumCorners[i] = xform * frustum.m_FrustumCorners[i];

        Matrix4 XForm = Transpose(Invert(Matrix4(xform)));

        for (int i = 0; i < 6; ++i)
            result.m_FrustumPlanes[i] = D_MATH_BOUNDS::BoundingPlane(XForm * Vector4(frustum.m_FrustumPlanes[i]));

        return result;
    }

    inline Frustum operator* (const Matrix4& mtx, const Frustum& frustum)
    {
        Frustum result;

        for (int i = 0; i < 8; ++i)
            result.m_FrustumCorners[i] = Vector3(mtx * frustum.m_FrustumCorners[i]);

        Matrix4 XForm = Transpose(Invert(mtx));

        for (int i = 0; i < 6; ++i)
            result.m_FrustumPlanes[i] = D_MATH_BOUNDS::BoundingPlane(XForm * Vector4(frustum.m_FrustumPlanes[i]));

        return result;
    }

} // namespace Math
