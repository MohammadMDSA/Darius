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
	class Vector3;
	class Vector4;
	//class Matrix4;
	class Quaternion;

	class DClass() Vector2
	{
	public:
		
		using ElementType = float;

		Vector2() : mData(0.f, 0.f) {}
		constexpr explicit Vector2(float ix) : mData(ix, ix) {}
		constexpr Vector2(float ix, float iy) : mData(ix, iy) {}
		explicit Vector2(_In_reads_(2) const float* pArray) : mData(pArray) {}
		explicit Vector2(DirectX::FXMVECTOR V) { DirectX::XMStoreFloat2(&mData, V); }
		explicit Vector2(const DirectX::XMFLOAT2 & V) { mData.x = V.x; mData.y = V.y; }
		explicit Vector2(const DirectX::XMVECTORF32 & F) { mData.x = F.f[0]; mData.y = F.f[1]; }

		Vector2(const Vector2&) = default;
		Vector2& operator=(const Vector2&) = default;

		Vector2(Vector2&&) = default;
		Vector2& operator=(Vector2&&) = default;

		operator DirectX::XMVECTOR() const { return XMLoadFloat2(&mData); }
		operator DirectX::XMFLOAT2 const&() const { return mData; }

		// Comparison operators
		bool operator == (const Vector2 & V) const;
		bool operator != (const Vector2 & V) const;

		// Assignment operators
		Vector2& operator= (const DirectX::XMVECTORF32 & F) { mData.x = F.f[0]; mData.y = F.f[1]; return *this; }
		Vector2& operator+= (const Vector2 & V);
		Vector2& operator-= (const Vector2 & V);
		Vector2& operator*= (const Vector2 & V);
		Vector2& operator*= (float S);
		Vector2& operator/= (float S);

		// Unary operators
		Vector2 operator+ () const { return Vector2(mData); }
		Vector2 operator- () const { return Vector2(-mData.x, -mData.y); }

		// Vector operations
		bool InBounds(const Vector2 & Bounds) const;

		float Length() const;
		float LengthSquared() const;

		float Dot(const Vector2 & V) const;
		void Cross(const Vector2 & V, Vector2 & result) const;
		Vector2 Cross(const Vector2 & V) const;

		Vector2 Normal() const;
		void Normalize();
		void Normalize(Vector2 & result) const;

		void Clamp(const Vector2 & vmin, const Vector2 & vmax);
		void Clamp(const Vector2 & vmin, const Vector2 & vmax, Vector2 & result) const;

		INLINE float GetX() const { return mData.x; }
		INLINE float GetY() const { return mData.y; }

		INLINE void SetX(float x) { mData.x = x; }
		INLINE void SetY(float y) { mData.y = y; }

		INLINE D_CONTAINERS::DVector<float> GetData() const { return { GetX(), GetY() }; }
		// Costy, don't use too often
		INLINE void SetData(D_CONTAINERS::DVector<float> data) { SetX(data[0]); SetY(data[1]); }

		// Static functions
		static float Distance(const Vector2 & v1, const Vector2 & v2);
		static float DistanceSquared(const Vector2 & v1, const Vector2 & v2);

		static void Min(const Vector2 & v1, const Vector2 & v2, Vector2 & result);
		static Vector2 Min(const Vector2 & v1, const Vector2 & v2);

		static void Max(const Vector2 & v1, const Vector2 & v2, Vector2 & result);
		static Vector2 Max(const Vector2 & v1, const Vector2 & v2);

		static void Lerp(const Vector2 & v1, const Vector2 & v2, float t, Vector2 & result);
		static Vector2 Lerp(const Vector2 & v1, const Vector2 & v2, float t);

		static void SmoothStep(const Vector2 & v1, const Vector2 & v2, float t, Vector2 & result);
		static Vector2 SmoothStep(const Vector2 & v1, const Vector2 & v2, float t);

		static void Barycentric(const Vector2 & v1, const Vector2 & v2, const Vector2 & v3, float f, float g, Vector2 & result);
		static Vector2 Barycentric(const Vector2 & v1, const Vector2 & v2, const Vector2 & v3, float f, float g);

		static void CatmullRom(const Vector2 & v1, const Vector2 & v2, const Vector2 & v3, const Vector2 & v4, float t, Vector2 & result);
		static Vector2 CatmullRom(const Vector2 & v1, const Vector2 & v2, const Vector2 & v3, const Vector2 & v4, float t);

		static void Hermite(const Vector2 & v1, const Vector2 & t1, const Vector2 & v2, const Vector2 & t2, float t, Vector2 & result);
		static Vector2 Hermite(const Vector2 & v1, const Vector2 & t1, const Vector2 & v2, const Vector2 & t2, float t);

		static void Reflect(const Vector2 & ivec, const Vector2 & nvec, Vector2 & result);
		static Vector2 Reflect(const Vector2 & ivec, const Vector2 & nvec);

		static void Refract(const Vector2 & ivec, const Vector2 & nvec, float refractionIndex, Vector2 & result);
		static Vector2 Refract(const Vector2 & ivec, const Vector2 & nvec, float refractionIndex);

		//static void Transform(const Vector2 & v, const Quaternion & quat, Vector2 & result);
		//static Vector2 Transform(const Vector2 & v, const Quaternion & quat);

		/*static void Transform(const Vector2 & v, const Matrix4 & m, Vector2 & result);
		static Vector2 Transform(const Vector2 & v, const Matrix4 & m);
		static void Transform(_In_reads_(count) const Vector2 * varray, size_t count, const Matrix4 & m, _Out_writes_(count) Vector2 * resultArray);

		static void Transform(const Vector2 & v, const Matrix4 & m, Vector4 & result);
		static void Transform(_In_reads_(count) const Vector2 * varray, size_t count, const Matrix4 & m, _Out_writes_(count) Vector4 * resultArray);

		static void TransformNormal(const Vector2 & v, const Matrix4& m, Vector2 & result);
		static Vector2 TransformNormal(const Vector2 & v, const Matrix4& m);
		static void TransformNormal(_In_reads_(count) const Vector2 * varray, size_t count, const Matrix4& m, _Out_writes_(count) Vector2 * resultArray);*/

		// Constants
		static const Vector2 Zero;
		static const Vector2 One;
		static const Vector2 UnitX;
		static const Vector2 UnitY;

	private:
		RTTR_REGISTRATION_FRIEND;
		DirectX::XMFLOAT2 mData;
	};

	// Binary operators
	Vector2 operator+ (const Vector2& V1, const Vector2& V2);
	Vector2 operator- (const Vector2& V1, const Vector2& V2);
	Vector2 operator* (const Vector2& V1, const Vector2& V2);
	Vector2 operator* (const Vector2& V, float S);
	Vector2 operator/ (const Vector2& V1, const Vector2& V2);
	Vector2 operator/ (const Vector2& V, float S);
	Vector2 operator* (float S, const Vector2& V);

	// A 3-vector with an unspecified fourth component.  Depending on the context, the W can be 0 or 1, but both are implicit.
	// The actual value of the fourth component is undefined for performance reasons.
	class DClass() Vector3
	{
	public:

		using ElementType = float;

		INLINE Vector3() { m_vec = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f); }
		INLINE Vector3(float _x, float _y, float _z) { m_vec = DirectX::XMVectorSet(_x, _y, _z, _z); }
		explicit INLINE Vector3(float const* data) : Vector3(DirectX::XMFLOAT3(data)) {}
		INLINE Vector3(DirectX::XMFLOAT3 const& v) { m_vec = DirectX::XMLoadFloat3(&v); }
		INLINE Vector3(const Vector3 & v) { m_vec = v; }
		INLINE Vector3(Scalar s) { m_vec = s; }
		INLINE explicit Vector3(Vector4 const vec);
		INLINE explicit Vector3(DirectX::FXMVECTOR const& vec) { m_vec = vec; }
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
		INLINE Vector3 Normal() const { return Vector3(DirectX::XMVector3Normalize(m_vec)); }
		INLINE void Normalize() { m_vec = DirectX::XMVector3Normalize(m_vec); }
		INLINE float Length() const { return Scalar(DirectX::XMVector3Length(m_vec)); }
		INLINE float LengthSquare() const { return Scalar(DirectX::XMVector3LengthSq(m_vec)); }
		INLINE bool	Equals(Vector3 const& other) const { return DirectX::XMVector3Equal(m_vec, other.m_vec); }
		INLINE bool NearEquals(Vector3 const& other, float const& epsilon = DirectX::g_XMEpsilon[0]) const { return DirectX::XMVector3NearEqual(m_vec, other.m_vec, Scalar(epsilon)); }
		INLINE bool IsNearZero(float epsilon) const { return NearEquals(Vector3::Zero, epsilon); }
		INLINE bool IsZero() const { return DirectX::XMVector3Equal(m_vec, DirectX::g_XMZero); }

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
		INLINE Vector3& operator *= (float v) { *this = *this * v; return *this; }
		INLINE Vector3& operator /= (Vector3 v) { *this = *this / v; return *this; }

		// Costy, don't use too often
		INLINE D_CONTAINERS::DVector<float> GetData() const { return { GetX(), GetY(), GetZ() }; }
		// Costy, don't use too often
		INLINE void SetData(D_CONTAINERS::DVector<float> data) { SetX(data[0]); SetY(data[1]); SetZ(data[2]); }

		INLINE operator DirectX::XMVECTOR const& () { return m_vec; }
		INLINE operator DirectX::XMFLOAT3& () { return *(DirectX::XMFLOAT3*)&m_vec; }

		INLINE friend Vector3 operator* (Scalar  v1, Vector3 v2) { return Vector3(v1) * v2; }
		INLINE friend Vector3 operator/ (Scalar  v1, Vector3 v2) { return Vector3(v1) / v2; }
		INLINE friend Vector3 operator* (float   v1, Vector3 v2) { return Scalar(v1) * v2; }
		INLINE friend Vector3 operator/ (float   v1, Vector3 v2) { return Scalar(v1) / v2; }

		static float Distance(Vector3 const& v1, Vector3 const& v2);

		static const Vector3 Up;
		static const Vector3 Down;
		static const Vector3 Left;
		static const Vector3 Right;
		static const Vector3 Forward;
		static const Vector3 Backward;
		static const Vector3 Zero;
		static const Vector3 One;
		static const Vector3 UnitX;
		static const Vector3 UnitY;
		static const Vector3 UnitZ;

	protected:
		RTTR_REGISTRATION_FRIEND;
		DirectX::XMVECTOR m_vec;

	};

	// A 4-vector, completely defined.
	class DClass() Vector4
	{
	public:
		
		using ElementType = float;

		INLINE Vector4() { m_vec = DirectX::XMVectorSet(0.f, 0.f, 0.f, 0.f); }
		INLINE Vector4(float _x, float _y, float _z, float _w) { m_vec = DirectX::XMVectorSet(_x, _y, _z, _w); }
		explicit INLINE Vector4(const float* data) : Vector4(DirectX::XMFLOAT4(data)) { }
		INLINE Vector4(DirectX::XMFLOAT4 const& v) { m_vec = DirectX::XMLoadFloat4(&v); }
		INLINE Vector4(Vector3 const& xyz, float w) { m_vec = DirectX::XMVectorSetW(xyz, w); }
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

		INLINE operator DirectX::XMVECTOR const& () const { return m_vec; }
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

		// Costy, don't use too often
		INLINE D_CONTAINERS::DVector<float> GetData() const { return { GetX(), GetY(), GetZ(), GetW() }; }
		// Costy, don't use too often
		INLINE void SetData(D_CONTAINERS::DVector<float> data) { SetX(data[0]); SetY(data[1]); SetZ(data[2]); SetW(data[3]); }
		INLINE float Length() const { return Scalar(DirectX::XMVector4Length(m_vec)); }
		INLINE float LengthSquare() const { return Scalar(DirectX::XMVector4LengthSq(m_vec)); }
		INLINE bool	Equals(Vector4 const& other) const { return DirectX::XMVector4Equal(m_vec, other.m_vec); }
		INLINE bool NearEquals(Vector4 const& other, float const& epsilon = DirectX::g_XMEpsilon[0]) const { return DirectX::XMVector4NearEqual(m_vec, other.m_vec, Scalar(epsilon)); }
		INLINE bool IsNearZero(float epsilon) const { return NearEquals(Vector4::Zero, epsilon); }
		INLINE bool IsZero() const { return DirectX::XMVector4Equal(m_vec, DirectX::g_XMZero); }

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
		static const Vector4 UnitX;
		static const Vector4 UnitY;
		static const Vector4 UnitZ;
		static const Vector4 UnitW;

	protected:
		RTTR_REGISTRATION_FRIEND;
		DirectX::XMVECTOR m_vec;
	};

	D_STATIC_ASSERT(sizeof(Vector3) == 16);
	D_STATIC_ASSERT(sizeof(Vector4) == 16);

	// Defined after Vector4 methods are declared
	INLINE Vector3::Vector3(Vector4 const vec) : m_vec((DirectX::XMVECTOR)vec)
	{
	}

	INLINE float Vector3::Distance(Vector3 const& v1, Vector3 const& v2)
	{
		return (v1 - v2).Length();
	}

	// For W != 1, divide XYZ by W.  If W == 0, do nothing
	INLINE Vector3 MakeHomogeneous(Vector4 v)
	{
		Scalar W = v.GetW();
		return Vector3(DirectX::XMVectorSelect(DirectX::XMVectorDivide(v, W), v, DirectX::XMVectorEqual(W, SplatZero())));
	}

#include "Vector2.inl"

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

	bool DrawDetails(D_MATH::Vector2& elem, Vector2 const& defaultValue = Vector2::Zero);
	bool DrawDetails(D_MATH::Vector3& elem, D_MATH::Vector3 const& defaultValue = Vector3::Zero, bool isColor = false);
	bool DrawDetails(D_MATH::Vector4& elem, Vector4 const& defaultValue = Vector4::Zero, bool isColor = false);

#endif // _D_EDITOR

}

File_Vector_GENERATED