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

#include "Rotator.hpp"

#include "Quaternion.generated.hpp"

namespace Darius::Math
{
	class DClass() Quaternion
	{

	public:
		INLINE explicit Quaternion() { m_vec = DirectX::XMQuaternionIdentity(); }
		INLINE explicit Quaternion(const Vector3 & axis, const Scalar & angle) { m_vec = DirectX::XMQuaternionRotationAxis(axis, angle); }
		INLINE Quaternion(float pitch, float yaw, float roll) { m_vec = DirectX::XMQuaternionRotationMatrix(DirectX::XMMatrixRotationZ(roll) * DirectX::XMMatrixRotationX(pitch) * DirectX::XMMatrixRotationY(yaw)); }
		INLINE Quaternion(float x, float y, float z, float w) { m_vec = DirectX::XMVectorSet(x, y, z, w); }
		INLINE explicit Quaternion(const DirectX::XMMATRIX & matrix) { m_vec = DirectX::XMQuaternionRotationMatrix(matrix); }
		INLINE explicit Quaternion(DirectX::FXMVECTOR vec) { m_vec = vec; }
		INLINE explicit Quaternion(EIdentityTag) { m_vec = DirectX::XMQuaternionIdentity(); }
		explicit Quaternion(Rotator const& rot);

		// Costy, don't use too often
		INLINE D_CONTAINERS::DVector<float> GetData() const { return { GetX(), GetY(), GetZ(), GetW() }; }
		// Costy, don't use too often
		INLINE void SetData(D_CONTAINERS::DVector<float> data) { SetX(data[0]); SetY(data[1]); SetZ(data[2]); SetW(data[3]); }

		INLINE bool Equals(Quaternion const& other) const { return DirectX::XMVector4Equal(m_vec, other); }

		Vector3 Angles() const;
		Rotator GetRotator() const;

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

		INLINE Vector4 Vector4() const { return D_MATH::Vector4(m_vec); }

		INLINE Quaternion Invert() const { return Quaternion(DirectX::XMQuaternionInverse(m_vec)); }

		// Unary operators
		INLINE Quaternion operator~ (void) const { return Quaternion(DirectX::XMQuaternionConjugate(m_vec)); }
		INLINE Quaternion operator- (void) const { return Quaternion(DirectX::XMVectorNegate(m_vec)); }
		INLINE Quaternion operator+ (void) const { return *this; }

		INLINE Vector3 operator* (Vector3 rhs) const { return Vector3(DirectX::XMVector3Rotate(rhs, m_vec)); }

		// Assignment operators
		INLINE Quaternion& operator= (Quaternion const& rhs) { m_vec = rhs; return *this; }
		INLINE Quaternion& operator*= (Quaternion const& rhs) { *this = *this * rhs; return *this; }
		INLINE Quaternion& operator+= (Quaternion const& rhs) { *this = *this + rhs; return *this; }
		INLINE Quaternion& operator-= (Quaternion const& rhs) { *this = *this - rhs; return *this; }
		INLINE Quaternion& operator*= (float s) { *this = *this * s; return *this; }
		INLINE Quaternion& operator/= (Quaternion const& rhs) { *this = *this / rhs; return *this; }

		// Binary operators
		Quaternion operator+ (Quaternion const& rhs) const;
		Quaternion operator- (Quaternion const& rhs) const;
		Quaternion operator* (Quaternion const& rhs) const;
		Quaternion operator/ (Quaternion const& rhs) const;
		Quaternion operator* (float rhs) const;

		INLINE bool operator== (Quaternion const& rhs) const { return Equals(rhs); }

		INLINE operator DirectX::XMVECTOR const& () const { return m_vec; }

		INLINE static Quaternion GetShortestArcBetweenTwoVector(Vector3 const& v1, Vector3 const& v2)
		{
			auto angle = DirectX::XMVectorGetX(DirectX::XMVector3AngleBetweenVectors(v1, v2));
			auto cross = Vector3(DirectX::XMVector3Cross(v1, v2));
			if (DirectX::XMVector3Equal(cross, DirectX::XMVectorZero()))
				return angle > DirectX::g_XMEpsilon[0] ? Quaternion::Inverted : Quaternion::Identity;

			return Quaternion(cross, angle);
		}

		INLINE static Quaternion FromForwardAndAngle(Vector3 const& forward, float angleDeg = 0.f)
		{
			return Quaternion(DirectX::XMQuaternionRotationNormal(forward, DirectX::XMConvertToRadians(angleDeg)));
		}

		INLINE static Quaternion FromRotator(Rotator const& rot) { return Quaternion(rot); }

		static const Quaternion Identity;
		static const Quaternion Inverted;

	protected:
		RTTR_REGISTRATION_FRIEND

			DirectX::XMVECTOR m_vec;

	};

	// Binary operators
	INLINE Quaternion Quaternion::operator+ (Quaternion const& rhs) const
	{
		using namespace DirectX;
		Quaternion R;

		R.m_vec = XMVectorAdd(m_vec, rhs.m_vec);
		return R;
	}

	INLINE Quaternion Quaternion::operator- (Quaternion const& rhs) const
	{
		using namespace DirectX;
		Quaternion R;
		R.m_vec = XMVectorSubtract(m_vec, rhs.m_vec);
		return R;
	}

	INLINE Quaternion Quaternion::operator* (Quaternion const& rhs) const
	{
		using namespace DirectX;
		Quaternion R;
		R.m_vec = XMQuaternionMultiply(m_vec, rhs.m_vec);
		return R;
	}

	INLINE Quaternion Quaternion::operator* (float rhs) const
	{
		using namespace DirectX;
		Quaternion R;
		R.m_vec = XMVectorScale(m_vec, rhs);
		return R;
	}

	INLINE Quaternion Quaternion::operator/ (Quaternion const& rhs) const
	{
		using namespace DirectX;

		Quaternion R;
		R.m_vec = XMQuaternionMultiply(m_vec, XMQuaternionInverse(rhs.m_vec));
		return R;
	}

	INLINE Quaternion operator* (float S, Quaternion const& Q)
	{
		using namespace DirectX;
		return Quaternion(XMVectorScale((DirectX::XMVECTOR const&)Q, S));
	}

	INLINE Quaternion Normalize(Quaternion q) { return Quaternion(DirectX::XMQuaternionNormalize(q)); }
	INLINE Quaternion Slerp(Quaternion a, Quaternion b, float t) { return Normalize(Quaternion(DirectX::XMQuaternionSlerp(a, b, t))); }
	INLINE Quaternion Lerp(Quaternion a, Quaternion b, float t) { return Normalize(Quaternion(DirectX::XMVectorLerp(a, b, t))); }

#ifdef _D_EDITOR

	bool DrawDetails(D_MATH::Quaternion& quat, Quaternion const& defaultValue);

#endif // _D_EDITOR

}

File_Quaternion_GENERATED