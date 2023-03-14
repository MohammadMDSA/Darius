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
    class BoundingSphere
    {
    public:
        BoundingSphere() {}
        BoundingSphere(float x, float y, float z, float r) : m_repr(x, y, z, r) {}
        BoundingSphere(const DirectX::XMFLOAT4* unaligned_array) : m_repr(*unaligned_array) {}
        BoundingSphere(Vector3 center, Scalar radius);
        BoundingSphere(EZeroTag) : m_repr(kZero) {}
        explicit BoundingSphere(const DirectX::XMVECTOR& v) : m_repr(v) {}
        explicit BoundingSphere(const DirectX::XMFLOAT4& f4) : m_repr(f4) {}
        explicit BoundingSphere(Vector4 sphere) : m_repr(sphere) {}
        explicit operator Vector4() const { return Vector4(m_repr); }

        Vector3 GetCenter(void) const { return Vector3(m_repr); }
        Scalar GetRadius(void) const { return m_repr.GetW(); }

        BoundingSphere Union(const BoundingSphere& rhs);

    private:

        Vector4 m_repr;
    };

    //=======================================================================================================
    // Inline implementations
    //

    inline BoundingSphere::BoundingSphere(Vector3 center, Scalar radius)
    {
        m_repr = Vector4(center);
        m_repr.SetW(radius);
    }

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