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
#include <DirectXMath.h>
#include "Common.hpp"
#include <Utils/Common.hpp>

#ifndef D_MATH
#define D_MATH Darius::Math
#endif // !D_MATH


namespace Darius::Math
{
    class Scalar
    {
    public:
        INLINE Scalar() { m_vec = DirectX::XMVectorReplicate(0.f); }
        INLINE Scalar(const Scalar& s) { m_vec = s; }
        INLINE Scalar(float f) { m_vec = DirectX::XMVectorReplicate(f); }
        INLINE explicit Scalar(DirectX::FXMVECTOR vec) { m_vec = vec; }
        INLINE explicit Scalar(EZeroTag) { m_vec = SplatZero(); }
        INLINE explicit Scalar(EIdentityTag) { m_vec = SplatOne(); }

        INLINE operator DirectX::XMVECTOR() const { return m_vec; }
        INLINE operator float() const { return DirectX::XMVectorGetX(m_vec); }

    private:
        DirectX::XMVECTOR m_vec;
    };

    INLINE Scalar operator- (Scalar s) { return Scalar(DirectX::XMVectorNegate(s)); }
    INLINE Scalar operator+ (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorAdd(s1, s2)); }
    INLINE Scalar operator- (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorSubtract(s1, s2)); }
    INLINE Scalar operator* (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorMultiply(s1, s2)); }
    INLINE Scalar operator/ (Scalar s1, Scalar s2) { return Scalar(DirectX::XMVectorDivide(s1, s2)); }
    INLINE Scalar operator+ (Scalar s1, float s2) { return s1 + Scalar(s2); }
    INLINE Scalar operator- (Scalar s1, float s2) { return s1 - Scalar(s2); }
    INLINE Scalar operator* (Scalar s1, float s2) { return s1 * Scalar(s2); }
    INLINE Scalar operator/ (Scalar s1, float s2) { return s1 / Scalar(s2); }
    INLINE Scalar operator+ (float s1, Scalar s2) { return Scalar(s1) + s2; }
    INLINE Scalar operator- (float s1, Scalar s2) { return Scalar(s1) - s2; }
    INLINE Scalar operator* (float s1, Scalar s2) { return Scalar(s1) * s2; }
    INLINE Scalar operator/ (float s1, Scalar s2) { return Scalar(s1) / s2; }

}