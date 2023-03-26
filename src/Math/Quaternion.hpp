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

#include "Quaternion.generated.hpp"

namespace Darius::Math
{
	class DClass() Quaternion
	{
	public:
		INLINE Quaternion() { m_vec = DirectX::XMQuaternionIdentity(); }
		INLINE Quaternion(const Vector3 & axis, const Scalar & angle) { m_vec = DirectX::XMQuaternionRotationAxis(axis, angle); }
		INLINE Quaternion(float pitch, float yaw, float roll) { m_vec = DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationZ(roll) * DirectX::XMMatrixRotationX(pitch) * DirectX::XMMatrixRotationY(yaw)); }
		INLINE Quaternion(float x, float y, float z, float w) { m_vec = DirectX::XMVectorSet(x, y, z, w); }
		INLINE explicit Quaternion(const DirectX::XMMATRIX & matrix) { m_vec = DirectX::XMQuaternionRotationMatrix(matrix); }
		INLINE explicit Quaternion(DirectX::FXMVECTOR vec) { m_vec = vec; }
		INLINE explicit Quaternion(EIdentityTag) { m_vec = DirectX::XMQuaternionIdentity(); }

		INLINE Vector3 Angles() const
		{
			Vector3 result;
			auto mat = DirectX::XMMatrixRotationQuaternion(m_vec);
			auto fmat = DirectX::XMFLOAT4X4((float*)&mat);

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
					result.SetX(-DirectX::XM_PIDIV2);
					result.SetZ(-atan2f(fmat._13, fmat._11));
					result.SetY(0.f);
				}
			}
			else
			{
				result.SetX(DirectX::XM_PIDIV2);
				result.SetZ(atan2f(fmat._13, fmat._11));
				result.SetY(0.f);
			}
			return -result;
		}
		INLINE float GetX() const { return Scalar(DirectX::XMVectorSplatX(m_vec)); }
		INLINE float GetY() const { return Scalar(DirectX::XMVectorSplatY(m_vec)); }
		INLINE float GetZ() const { return Scalar(DirectX::XMVectorSplatZ(m_vec)); }
		INLINE float GetW() const { return Scalar(DirectX::XMVectorSplatW(m_vec)); }

		INLINE void SetX(float _x) { m_vec = DirectX::XMVectorPermute<4, 1, 2, 3>(m_vec, Scalar(_x)); }
		INLINE void SetY(float _y) { m_vec = DirectX::XMVectorPermute<0, 5, 2, 3>(m_vec, Scalar(_y)); }
		INLINE void SetZ(float _z) { m_vec = DirectX::XMVectorPermute<0, 1, 6, 3>(m_vec, Scalar(_z)); }
		INLINE void SetW(float _w) { m_vec = DirectX::XMVectorPermute<0, 1, 2, 7>(m_vec, Scalar(_w)); }

		INLINE Vector3 GetForward() const { return *this * Vector3::Forward; }
		INLINE Vector3 GetRight() const { return *this * Vector3::Right; }
		INLINE Vector3 GetUp() const { return *this * Vector3::Up; }

		INLINE Quaternion operator~ (void) const { return Quaternion(DirectX::XMQuaternionConjugate(m_vec)); }
		INLINE Quaternion operator- (void) const { return Quaternion(DirectX::XMVectorNegate(m_vec)); }

		INLINE Quaternion operator* (Quaternion rhs) const { return Quaternion(DirectX::XMQuaternionMultiply(rhs, m_vec)); }
		INLINE Vector3 operator* (Vector3 rhs) const { return Vector3(DirectX::XMVector3Rotate(rhs, m_vec)); }

		INLINE Quaternion& operator= (Quaternion rhs) { m_vec = rhs; return *this; }
		INLINE Quaternion& operator*= (Quaternion rhs) { *this = *this * rhs; return *this; }

		INLINE operator DirectX::XMVECTOR const& () const { return m_vec; }

		static const Quaternion Identity;

	protected:
		RTTR_REGISTRATION_FRIEND

		DirectX::XMVECTOR m_vec;

	public:
		Darius_Math_Quaternion_GENERATED
	};

	INLINE Quaternion Normalize(Quaternion q) { return Quaternion(DirectX::XMQuaternionNormalize(q)); }
	INLINE Quaternion Slerp(Quaternion a, Quaternion b, float t) { return Normalize(Quaternion(DirectX::XMQuaternionSlerp(a, b, t))); }
	INLINE Quaternion Lerp(Quaternion a, Quaternion b, float t) { return Normalize(Quaternion(DirectX::XMVectorLerp(a, b, t))); }


#ifdef _D_EDITOR

	bool DrawDetails(D_MATH::Quaternion& quat, float params[]);

#endif // _D_EDITOR

}

File_Quaternion_GENERATED