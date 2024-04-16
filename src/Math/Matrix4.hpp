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

#include "Common.hpp"
#include "Transform.hpp"
#include "Color.hpp"

namespace Darius::Math
{
	__declspec(align(16)) class Matrix4
	{
	public:
		INLINE Matrix4() {
			m_mat.r[0] = SetWToZero(Vector3()); m_mat.r[1] = SetWToZero(Vector3());
			m_mat.r[2] = SetWToZero(Vector3()); m_mat.r[3] = SetWToOne(Vector3());
		}
		INLINE Matrix4(Vector3 x, Vector3 y, Vector3 z, Vector3 w)
		{
			m_mat.r[0] = SetWToZero(x); m_mat.r[1] = SetWToZero(y);
			m_mat.r[2] = SetWToZero(z); m_mat.r[3] = SetWToOne(w);
		}

		INLINE Matrix4(const float* data)
		{
			m_mat = DirectX::XMLoadFloat4x4((DirectX::XMFLOAT4X4*)data);
		}

		INLINE Matrix4(Vector4 x, Vector4 y, Vector4 z, Vector4 w) { m_mat.r[0] = x; m_mat.r[1] = y; m_mat.r[2] = z; m_mat.r[3] = w; }
		INLINE Matrix4(const Matrix4& mat) { m_mat = mat.m_mat; }
		INLINE Matrix4(const Matrix3& mat)
		{
			m_mat.r[0] = SetWToZero(mat.GetX());
			m_mat.r[1] = SetWToZero(mat.GetY());
			m_mat.r[2] = SetWToZero(mat.GetZ());
			m_mat.r[3] = CreateWUnitVector();
		}
		INLINE Matrix4(const Matrix3& xyz, Vector3 w)
		{
			m_mat.r[0] = SetWToZero(xyz.GetX());
			m_mat.r[1] = SetWToZero(xyz.GetY());
			m_mat.r[2] = SetWToZero(xyz.GetZ());
			m_mat.r[3] = SetWToOne(w);
		}
		INLINE Matrix4(const AffineTransform& xform) { *this = Matrix4(xform.GetBasis(), xform.GetTranslation()); }
		INLINE Matrix4(const OrthogonalTransform& xform) { *this = Matrix4(Matrix3(xform.GetRotation()), xform.GetTranslation()); }
		INLINE explicit Matrix4(const DirectX::XMMATRIX& mat) { m_mat = mat; }
		INLINE explicit Matrix4(EIdentityTag) { m_mat = DirectX::XMMatrixIdentity(); }
		INLINE explicit Matrix4(EZeroTag) { m_mat.r[0] = m_mat.r[1] = m_mat.r[2] = m_mat.r[3] = SplatZero(); }

		INLINE const Matrix3& Get3x3() const { return (const Matrix3&)*this; }
		INLINE void Set3x3(const Matrix3& xyz)
		{
			m_mat.r[0] = SetWToZero(xyz.GetX());
			m_mat.r[1] = SetWToZero(xyz.GetY());
			m_mat.r[2] = SetWToZero(xyz.GetZ());
		}

		INLINE Vector4 GetX() const { return Vector4(m_mat.r[0]); }
		INLINE Vector4 GetY() const { return Vector4(m_mat.r[1]); }
		INLINE Vector4 GetZ() const { return Vector4(m_mat.r[2]); }
		INLINE Vector4 GetW() const { return Vector4(m_mat.r[3]); }

		INLINE void SetX(Vector4 x) { m_mat.r[0] = x; }
		INLINE void SetY(Vector4 y) { m_mat.r[1] = y; }
		INLINE void SetZ(Vector4 z) { m_mat.r[2] = z; }
		INLINE void SetW(Vector4 w) { m_mat.r[3] = w; }

		INLINE Matrix4 Transpose() const { return Matrix4(DirectX::XMMatrixTranspose(m_mat)); }
		INLINE Matrix4 Inverse() const { auto det = DirectX::XMMatrixDeterminant(m_mat); return Matrix4(DirectX::XMMatrixInverse(&det, m_mat)); }

		// Consty, don't use too often
		INLINE D_CONTAINERS::DVector<D_CONTAINERS::DVector<float>> GetData() const
		{
			auto x = GetX();
			auto y = GetY();
			auto z = GetZ();
			auto w = GetW();
			return {
					{ x.GetX(), x.GetY(), x.GetZ(), x.GetW() },
					{ y.GetX(), y.GetY(), y.GetZ(), y.GetW() },
					{ z.GetX(), z.GetY(), z.GetZ(),	z.GetW() },
					{ w.GetX(), w.GetY(), w.GetZ(),	w.GetW() }
			};
		}

		// Consty, don't use too often
		INLINE void SetData(D_CONTAINERS::DVector<D_CONTAINERS::DVector<float>> data)
		{
			m_mat.r[0] = Vector4(data[0].data());
			m_mat.r[1] = Vector4(data[1].data());
			m_mat.r[2] = Vector4(data[2].data());
			m_mat.r[3] = Vector4(data[3].data());
		}

		INLINE operator DirectX::XMMATRIX() const { return m_mat; }
		INLINE operator DirectX::XMFLOAT4X4() const { DirectX::XMFLOAT4X4 dest; DirectX::XMStoreFloat4x4(&dest, m_mat); return dest; }

		INLINE Vector4 operator* (Vector3 const& vec) const { return Vector4(DirectX::XMVector3Transform(vec, m_mat)); }
		INLINE Vector4 operator* (Vector4 const& vec) const { return Vector4(DirectX::XMVector4Transform(vec, m_mat)); }
		INLINE Color operator* (Color const& col) const { return Color(DirectX::XMVector4Transform(col, m_mat)); }
		INLINE Matrix4 operator* (const Matrix4& mat) const { return Matrix4(DirectX::XMMatrixMultiply(mat, m_mat)); }

		INLINE float GetElement(int row, int col) const { return m_mat.r[row].m128_f32[col]; }
		INLINE float& GetElement(int row, int col) { return m_mat.r[row].m128_f32[col]; }
		INLINE Vector4 GetColumn(int col) const { D_ASSERT(col >= 0 && col < 3); return Vector4(m_mat.r[0].m128_f32[col], m_mat.r[1].m128_f32[col], m_mat.r[2].m128_f32[col], m_mat.r[3].m128_f32[col]); }

		static INLINE Matrix4 MakeScale(float scale) { return Matrix4(DirectX::XMMatrixScaling(scale, scale, scale)); }
		static INLINE Matrix4 MakeScale(Vector3 scale) { return Matrix4(DirectX::XMMatrixScalingFromVector(scale)); }
		static INLINE Matrix4 MakeLookAt(Vector3 eyePos, Vector3 target, Vector3 up) { return Matrix4(DirectX::XMMatrixLookAtRH(eyePos, target, up)); }
		static INLINE Matrix4 MakeLookToward(Vector3 eyePos, Vector3 dir, Vector3 up) { return Matrix4(DirectX::XMMatrixLookToRH(eyePos, dir, up)); }
		static INLINE Matrix4 MakeProjection(float fov, float ratio, float nearP, float farP) { return Matrix4(DirectX::XMMatrixPerspectiveFovRH(fov, ratio, nearP, farP)); }
		static INLINE Matrix4 MakeTranslation(Vector3 trans) { return Matrix4(DirectX::XMMatrixTranslationFromVector(trans)); }
		static INLINE Matrix4 MakeTranslation(float x, float y, float z) { return Matrix4(DirectX::XMMatrixTranslation(x, y, z)); }
		static INLINE Matrix4 Inverse(Matrix4 const& mat) { return Matrix4(DirectX::XMMatrixInverse(nullptr, mat)); }
		static INLINE Matrix4 Transpose(Matrix4 const& mat) { return Matrix4(DirectX::XMMatrixTranspose(mat.m_mat)); }

		static const Matrix4 Identity;
		static const Matrix4 Zero;

	private:
		DirectX::XMMATRIX m_mat;
	};
}
