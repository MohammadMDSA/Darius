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
#include "Vector.hpp"

namespace Darius::Math
{
    class Quaternion
    {
    public:
        INLINE Quaternion() { m_vec = XMQuaternionIdentity(); }
        INLINE Quaternion(const Vector3& axis, const Scalar& angle) { m_vec = XMQuaternionRotationAxis(axis, angle); }
        INLINE Quaternion(Vector3 const& normal, float const& angle) { m_vec = XMQuaternionRotationNormal(normal, angle); }
        INLINE Quaternion(float pitch, float yaw, float roll) { m_vec = XMQuaternionRotationMatrix(XMMatrixRotationZ(roll) * XMMatrixRotationX(pitch) * XMMatrixRotationY(yaw)); }
        INLINE Quaternion(float x, float y, float z, float w) { m_vec = XMVectorSet(x, y, z, w); }
        INLINE explicit Quaternion(const XMMATRIX& matrix) { m_vec = XMQuaternionRotationMatrix(matrix); }
        INLINE explicit Quaternion(FXMVECTOR vec) { m_vec = vec; }
        INLINE explicit Quaternion(EIdentityTag) { m_vec = XMQuaternionIdentity(); }
        
        INLINE Vector3 Angles() const
        {
            Vector3 result;
            auto mat = XMMatrixRotationQuaternion(m_vec);
            auto fmat = XMFLOAT4X4((float*)&mat);

            if (fmat._32 < 1.f)
            {
                if (fmat._32 > -1.f)
                {
                    result.SetX(asinf(fmat._32));
                    result.SetZ(atan2f(-fmat._12, fmat._22));
                    result.SetY(atan2f(-fmat._31, fmat._33));
                }
                else
                {
                    result.SetX(-XM_PIDIV2);
                    result.SetZ(-atan2f(fmat._13, fmat._11));
                    result.SetY(0.f);
                }
            }
            else
            {
                result.SetX(XM_PIDIV2);
                result.SetZ(atan2f(fmat._13, fmat._11));
                result.SetY(0.f);
            }
            return -result;
        }
        INLINE Scalar GetX() const { return Scalar(XMVectorSplatX(m_vec)); }
        INLINE Scalar GetY() const { return Scalar(XMVectorSplatY(m_vec)); }
        INLINE Scalar GetZ() const { return Scalar(XMVectorSplatZ(m_vec)); }
        INLINE Scalar GetW() const { return Scalar(XMVectorSplatW(m_vec)); }

        INLINE Vector3 GetForward() const { return *this * Vector3::Forward(); }
        INLINE Vector3 GetRight() const { return *this * Vector3::Right(); }
        INLINE Vector3 GetUp() const { return *this * Vector3::Up(); }

        INLINE Quaternion operator~ (void) const { return Quaternion(XMQuaternionConjugate(m_vec)); }
        INLINE Quaternion operator- (void) const { return Quaternion(XMVectorNegate(m_vec)); }

        INLINE Quaternion operator* (Quaternion rhs) const { return Quaternion(XMQuaternionMultiply(rhs, m_vec)); }
        INLINE Vector3 operator* (Vector3 rhs) const { return Vector3(XMVector3Rotate(rhs, m_vec)); }

        INLINE Quaternion& operator= (Quaternion rhs) { m_vec = rhs; return *this; }
        INLINE Quaternion& operator*= (Quaternion rhs) { *this = *this * rhs; return *this; }

        INLINE operator XMVECTOR () const { return m_vec; }
        INLINE operator Vector3& () { return *(Vector3*)&m_vec; }

    protected:
        XMVECTOR m_vec;
    };

    INLINE Quaternion Normalize(Quaternion q) { return Quaternion(XMQuaternionNormalize(q)); }
    INLINE Quaternion Slerp(Quaternion a, Quaternion b, float t) { return Normalize(Quaternion(XMQuaternionSlerp(a, b, t))); }
    INLINE Quaternion Lerp(Quaternion a, Quaternion b, float t) { return Normalize(Quaternion(XMVectorLerp(a, b, t))); }


#ifdef _D_EDITOR

    bool DrawDetails(D_MATH::Quaternion& quat, float params[]);

#endif // _D_EDITOR

}