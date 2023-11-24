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

#include "Scalar.hpp"

#include <Core/Containers/Vector.hpp>

#include "Vector.generated.hpp"

namespace Darius::Math
{
	class Vector4;

	// A 3-vector with an unspecified fourth component.  Depending on the context, the W can be 0 or 1, but both are implicit.
	// The actual value of the fourth component is undefined for performance reasons.
	class DClass() Vector3
	{
	public:

		INLINE Vector3() { m_vec = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f); }
		INLINE Vector3(float _x, float _y, float _z) { m_vec = DirectX::XMVectorSet(_x, _y, _z, _z); }
		explicit INLINE Vector3(const float* data) : Vector3(DirectX::XMFLOAT3(data)) {}
		INLINE Vector3(const DirectX::XMFLOAT3 & v) { m_vec = DirectX::XMLoadFloat3(&v); }
		INLINE Vector3(const Vector3 & v) { m_vec = v; }
		INLINE Vector3(Scalar s) { m_vec = s; }
		INLINE explicit Vector3(Vector4 vec);
		INLINE explicit Vector3(DirectX::FXMVECTOR vec) { m_vec = vec; }
		INLINE explicit Vector3(EZeroTag) { m_vec = SplatZero(); }
		INLINE explicit Vector3(EIdentityTag) { m_vec = SplatOne(); }
		INLINE explicit Vector3(EXUnitVector) { m_vec = CreateXUnitVector(); }
		INLINE explicit Vector3(EYUnitVector) { m_vec = CreateYUnitVector(); }
		INLINE explicit Vector3(EZUnitVector) { m_vec = CreateZUnitVector(); }

		INLINE operator DirectX::XMVECTOR const& () const { return m_vec; }
		INLINE operator DirectX::XMFLOAT3() const { DirectX::XMFLOAT3 dest; DirectX::XMStoreFloat3(&dest, m_vec); return dest; }

		INLINE float GetX() const { return Scalar(DirectX::XMVectorSplatX(m_vec)); }
		INLINE float GetY() const { return Scalar(DirectX::XMVectorSplatY(m_vec)); }
		INLINE float GetZ() const { return Scalar(DirectX::XMVectorSplatZ(m_vec)); }
		INLINE void SetX(float _x) { m_vec = DirectX::XMVectorPermute<4, 1, 2, 3>(m_vec, Scalar(_x)); }
		INLINE void SetY(float _y) { m_vec = DirectX::XMVectorPermute<0, 5, 2, 3>(m_vec, Scalar(_y)); }
		INLINE void SetZ(float _z) { m_vec = DirectX::XMVectorPermute<0, 1, 6, 3>(m_vec, Scalar(_z)); }
		INLINE Vector3 Normalize() const { return Vector3(DirectX::XMVector3Normalize(m_vec)); }
		INLINE float Length() const { return Scalar(DirectX::XMVector3Length(m_vec)); }
		INLINE float LengthSquare() const { return Scalar(DirectX::XMVector3LengthSq(m_vec)); }
		INLINE bool	Equals(Vector3 const& other) const { return DirectX::XMVector3Equal(m_vec, other.m_vec); }

		INLINE Vector3 operator- () const { return Vector3(DirectX::XMVectorNegate(m_vec)); }
		INLINE Vector3 operator+ (Vector3 v2) const { return Vector3(DirectX::XMVectorAdd(m_vec, v2)); }
		INLINE Vector3 operator- (Vector3 v2) const { return Vector3(DirectX::XMVectorSubtract(m_vec, v2)); }
		INLINE Vector3 operator* (Vector3 v2) const { return Vector3(DirectX::XMVectorMultiply(m_vec, v2)); }
		INLINE Vector3 operator/ (Vector3 v2) const { return Vector3(DirectX::XMVectorDivide(m_vec, v2)); }
		INLINE Vector3 operator* (Scalar  v2) const { return *this * Vector3(v2); }
		INLINE Vector3 operator/ (Scalar  v2) const { return *this / Vector3(v2); }
		INLINE Vector3 operator* (float  v2) const { return *this * Scalar(v2); }
		INLINE Vector3 operator/ (float  v2) const { return *this / Scalar(v2); }

		INLINE Vector3& operator += (Vector3 v) { *this = *this + v; return *this; }
		INLINE Vector3& operator -= (Vector3 v) { *this = *this - v; return *this; }
		INLINE Vector3& operator *= (Vector3 v) { *this = *this * v; return *this; }
		INLINE Vector3& operator /= (Vector3 v) { *this = *this / v; return *this; }

		// Consty, don't use too often
		INLINE D_CONTAINERS::DVector<float> GetData() const { return { GetX(), GetY(), GetZ() }; }
		// Consty, don't use too often
		INLINE void SetData(D_CONTAINERS::DVector<float> data) { SetX(data[0]); SetY(data[1]); SetZ(data[2]); }

		INLINE operator DirectX::XMVECTOR const& () { return m_vec; }
		INLINE operator DirectX::XMFLOAT3& () { return *(DirectX::XMFLOAT3*)&m_vec; }

		INLINE friend Vector3 operator* (Scalar  v1, Vector3 v2) { return Vector3(v1) * v2; }
		INLINE friend Vector3 operator/ (Scalar  v1, Vector3 v2) { return Vector3(v1) / v2; }
		INLINE friend Vector3 operator* (float   v1, Vector3 v2) { return Scalar(v1) * v2; }
		INLINE friend Vector3 operator/ (float   v1, Vector3 v2) { return Scalar(v1) / v2; }

		static const Vector3 Up;
		static const Vector3 Down;
		static const Vector3 Left;
		static const Vector3 Right;
		static const Vector3 Forward;
		static const Vector3 Backward;
		static const Vector3 Zero;
		static const Vector3 One;

	protected:
		RTTR_REGISTRATION_FRIEND;
		DirectX::XMVECTOR m_vec;

	};

	// A 4-vector, completely defined.
	class DClass() Vector4
	{
	public:
		INLINE Vector4() { m_vec = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f); }
		INLINE Vector4(float _x, float _y, float _z, float _w) { m_vec = DirectX::XMVectorSet(_x, _y, _z, _w); }
		explicit INLINE Vector4(const float* data) : Vector4(DirectX::XMFLOAT4(data)) { }
		INLINE Vector4(const DirectX::XMFLOAT4 & v) { m_vec = DirectX::XMLoadFloat4(&v); }
		INLINE Vector4(Vector3 xyz, float w) { m_vec = DirectX::XMVectorSetW(xyz, w); }
		INLINE Vector4(const Vector4 & v) { m_vec = v; }
		INLINE Vector4(const Scalar & s) { m_vec = s; }
		INLINE explicit Vector4(Vector3 xyz) { m_vec = SetWToOne(xyz); }
		INLINE explicit Vector4(DirectX::FXMVECTOR vec) { m_vec = vec; }
		INLINE explicit Vector4(EZeroTag) { m_vec = SplatZero(); }
		INLINE explicit Vector4(EIdentityTag) { m_vec = SplatOne(); }
		INLINE explicit Vector4(EXUnitVector) { m_vec = CreateXUnitVector(); }
		INLINE explicit Vector4(EYUnitVector) { m_vec = CreateYUnitVector(); }
		INLINE explicit Vector4(EZUnitVector) { m_vec = CreateZUnitVector(); }
		INLINE explicit Vector4(EWUnitVector) { m_vec = CreateWUnitVector(); }

		INLINE operator DirectX::XMVECTOR const&() const { return m_vec; }
		INLINE operator DirectX::XMFLOAT4() const { DirectX::XMFLOAT4 dest; DirectX::XMStoreFloat4(&dest, m_vec); return dest; }

		INLINE float GetX() const { return Scalar(DirectX::XMVectorSplatX(m_vec)); }
		INLINE float GetY() const { return Scalar(DirectX::XMVectorSplatY(m_vec)); }
		INLINE float GetZ() const { return Scalar(DirectX::XMVectorSplatZ(m_vec)); }
		INLINE float GetW() const { return Scalar(DirectX::XMVectorSplatW(m_vec)); }
		INLINE void SetX(float _x) { m_vec = DirectX::XMVectorPermute<4, 1, 2, 3>(m_vec, Scalar(_x)); }
		INLINE void SetY(float _y) { m_vec = DirectX::XMVectorPermute<0, 5, 2, 3>(m_vec, Scalar(_y)); }
		INLINE void SetZ(float _z) { m_vec = DirectX::XMVectorPermute<0, 1, 6, 3>(m_vec, Scalar(_z)); }
		INLINE void SetW(float _w) { m_vec = DirectX::XMVectorPermute<0, 1, 2, 7>(m_vec, Scalar(_w)); }
		INLINE void SetXYZ(Vector3 xyz) { m_vec = DirectX::XMVectorPermute<0, 1, 2, 7>(xyz, m_vec); }

		// Consty, don't use too often
		INLINE D_CONTAINERS::DVector<float> GetData() const { return { GetX(), GetY(), GetZ(), GetW() }; }
		// Consty, don't use too often
		INLINE void SetData(D_CONTAINERS::DVector<float> data) { SetX(data[0]); SetY(data[1]); SetZ(data[2]); SetW(data[3]); }
		INLINE float Length() const { return Scalar(DirectX::XMVector4Length(m_vec)); }
		INLINE float LengthSquare() const { return Scalar(DirectX::XMVector4LengthSq(m_vec)); }
		INLINE bool	Equals(Vector4 const& other) const { return DirectX::XMVector4Equal(m_vec, other.m_vec); }

		INLINE Vector4 operator- () const { return Vector4(DirectX::XMVectorNegate(m_vec)); }
		INLINE Vector4 operator+ (Vector4 v2) const { return Vector4(DirectX::XMVectorAdd(m_vec, v2)); }
		INLINE Vector4 operator- (Vector4 v2) const { return Vector4(DirectX::XMVectorSubtract(m_vec, v2)); }
		INLINE Vector4 operator* (Vector4 v2) const { return Vector4(DirectX::XMVectorMultiply(m_vec, v2)); }
		INLINE Vector4 operator/ (Vector4 v2) const { return Vector4(DirectX::XMVectorDivide(m_vec, v2)); }
		INLINE Vector4 operator* (Scalar  v2) const { return *this * Vector4(v2); }
		INLINE Vector4 operator/ (Scalar  v2) const { return *this / Vector4(v2); }
		INLINE Vector4 operator* (float   v2) const { return *this * Scalar(v2); }
		INLINE Vector4 operator/ (float   v2) const { return *this / Scalar(v2); }

		INLINE void operator*= (float   v2) { *this = *this * Scalar(v2); }
		INLINE void operator/= (float   v2) { *this = *this / Scalar(v2); }

		INLINE friend Vector4 operator* (Scalar  v1, Vector4 v2) { return Vector4(v1) * v2; }
		INLINE friend Vector4 operator/ (Scalar  v1, Vector4 v2) { return Vector4(v1) / v2; }
		INLINE friend Vector4 operator* (float   v1, Vector4 v2) { return Scalar(v1) * v2; }
		INLINE friend Vector4 operator/ (float   v1, Vector4 v2) { return Scalar(v1) / v2; }

		static const Vector4 Up;
		static const Vector4 Down;
		static const Vector4 Left;
		static const Vector4 Right;
		static const Vector4 Forward;
		static const Vector4 Backward;
		static const Vector4 Zero;
		static const Vector4 One;

	protected:
		RTTR_REGISTRATION_FRIEND;
		DirectX::XMVECTOR m_vec;
	};

	D_STATIC_ASSERT(sizeof(Vector3) == 16);
	D_STATIC_ASSERT(sizeof(Vector4) == 16);

	// Defined after Vector4 methods are declared
	INLINE Vector3::Vector3(Vector4 vec) : m_vec((DirectX::XMVECTOR)vec)
	{
	}

	// For W != 1, divide XYZ by W.  If W == 0, do nothing
	INLINE Vector3 MakeHomogeneous(Vector4 v)
	{
		Scalar W = v.GetW();
		return Vector3(DirectX::XMVectorSelect(DirectX::XMVectorDivide(v, W), v, DirectX::XMVectorEqual(W, SplatZero())));
	}

	class BoolVector
	{
	public:
		INLINE BoolVector(DirectX::FXMVECTOR vec) { m_vec = vec; }

		INLINE bool All() const { return DirectX::XMVector4Equal(m_vec, DirectX::g_XMOne); }
		INLINE bool All3() const { return DirectX::XMVector3Equal(m_vec, DirectX::g_XMOne); }
		INLINE operator DirectX::XMVECTOR() const { return m_vec; }
		INLINE operator bool() const { return All(); }
	protected:
		DirectX::XMVECTOR m_vec;

	};

#ifdef _D_EDITOR

#define D_H_DRAW_DETAILS_MAKE_VEC_PARAM(_default, hasColor) { _default, hasColor ? 1 : 0 }
#define D_H_DRAW_DETAILS_MAKE_VEC_PARAM_COLOR D_H_DRAW_DETAILS_MAKE_VEC_PARAM(0, 1)
#define D_H_DRAW_DETAILS_MAKE_VEC_PARAM_VECTOR D_H_DRAW_DETAILS_MAKE_VEC_PARAM(0, 0)

	bool DrawDetails(D_MATH::Vector3& elem, float params[]);

	bool DrawDetails(D_MATH::Vector4& elem, float params[]);

#endif // _D_EDITOR

}

File_Vector_GENERATED