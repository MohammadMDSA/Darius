#pragma once
#include <Utils/Common.hpp>

#pragma warning(push)
#pragma warning(disable:4244)

namespace Darius::Math
{
	// To allow floats to implicitly construct Scalars, we need to clarify these operators and suppress
	// upconversion.
	INLINE bool operator<  (Scalar lhs, float rhs) { return (float)lhs < rhs; }
	INLINE bool operator<= (Scalar lhs, float rhs) { return (float)lhs <= rhs; }
	INLINE bool operator>  (Scalar lhs, float rhs) { return (float)lhs > rhs; }
	INLINE bool operator>= (Scalar lhs, float rhs) { return (float)lhs >= rhs; }
	INLINE bool operator== (Scalar lhs, float rhs) { return (float)lhs == rhs; }
	INLINE bool operator<  (float lhs, Scalar rhs) { return lhs < (float)rhs; }
	INLINE bool operator<= (float lhs, Scalar rhs) { return lhs <= (float)rhs; }
	INLINE bool operator>  (float lhs, Scalar rhs) { return lhs > (float)rhs; }
	INLINE bool operator>= (float lhs, Scalar rhs) { return lhs >= (float)rhs; }
	INLINE bool operator== (float lhs, Scalar rhs) { return lhs == (float)rhs; }

#define CREATE_SIMD_FUNCTIONS( TYPE ) \
	INLINE TYPE Sqrt( TYPE const& s ) { return TYPE(DirectX::XMVectorSqrt(s)); } \
	INLINE TYPE Recip( TYPE const& s ) { return TYPE(DirectX::XMVectorReciprocal(s)); } \
	INLINE TYPE RecipSqrt( TYPE const& s ) { return TYPE(DirectX::XMVectorReciprocalSqrt(s)); } \
	INLINE TYPE Floor( TYPE const& s ) { return TYPE(DirectX::XMVectorFloor(s)); } \
	INLINE TYPE Ceiling( TYPE const& s ) { return TYPE(DirectX::XMVectorCeiling(s)); } \
	INLINE TYPE Round( TYPE const& s ) { return TYPE(DirectX::XMVectorRound(s)); } \
	INLINE TYPE Abs( TYPE const& s ) { return TYPE(DirectX::XMVectorAbs(s)); } \
	INLINE TYPE Exp( TYPE const& s ) { return TYPE(DirectX::XMVectorExp(s)); } \
	INLINE TYPE Pow( TYPE const& b, TYPE const& e ) { return TYPE(DirectX::XMVectorPow(b, e)); } \
	INLINE TYPE Log( TYPE const& s ) { return TYPE(DirectX::XMVectorLog(s)); } \
	INLINE TYPE Sin( TYPE const& s ) { return TYPE(DirectX::XMVectorSin(s)); } \
	INLINE TYPE Cos( TYPE const& s ) { return TYPE(DirectX::XMVectorCos(s)); } \
	INLINE TYPE Tan( TYPE const& s ) { return TYPE(DirectX::XMVectorTan(s)); } \
	INLINE TYPE ASin( TYPE const& s ) { return TYPE(DirectX::XMVectorASin(s)); } \
	INLINE TYPE ACos( TYPE const& s ) { return TYPE(DirectX::XMVectorACos(s)); } \
	INLINE TYPE ATan( TYPE const& s ) { return TYPE(DirectX::XMVectorATan(s)); } \
	INLINE TYPE ATan2( TYPE const& y, TYPE const& x ) { return TYPE(DirectX::XMVectorATan2(y, x)); } \
	INLINE TYPE Lerp( TYPE const& a, TYPE const& b, TYPE const& t ) { return TYPE(DirectX::XMVectorLerpV(a, b, t)); } \
    INLINE TYPE Lerp( TYPE const& a, TYPE const& b, float t ) { return TYPE(DirectX::XMVectorLerp(a, b, t)); } \
	INLINE TYPE Max( TYPE const& a, TYPE const& b ) { return TYPE(DirectX::XMVectorMax(a, b)); } \
	INLINE TYPE Min( TYPE const& a, TYPE const& b ) { return TYPE(DirectX::XMVectorMin(a, b)); } \
	INLINE TYPE Clamp( TYPE const& v, TYPE const& a, TYPE const& b ) { return Min(Max(v, a), b); } \
	INLINE BoolVector operator<  ( TYPE const& lhs, TYPE const& rhs ) { return DirectX::XMVectorLess(lhs, rhs); } \
	INLINE BoolVector operator<= ( TYPE const& lhs, TYPE const& rhs ) { return DirectX::XMVectorLessOrEqual(lhs, rhs); } \
	INLINE BoolVector operator>  ( TYPE const& lhs, TYPE const& rhs ) { return DirectX::XMVectorGreater(lhs, rhs); } \
	INLINE BoolVector operator>= ( TYPE const& lhs, TYPE const& rhs ) { return DirectX::XMVectorGreaterOrEqual(lhs, rhs); } \
	INLINE TYPE Select( TYPE const& lhs, TYPE const& rhs, BoolVector const& mask ) { return TYPE(DirectX::XMVectorSelect(lhs, rhs, mask)); } \
	//INLINE BoolVector operator== ( TYPE const& lhs, TYPE const& rhs ) { return DirectX::XMVectorEqual(lhs, rhs); }

	CREATE_SIMD_FUNCTIONS(Scalar);
	CREATE_SIMD_FUNCTIONS(Vector3);
	CREATE_SIMD_FUNCTIONS(Vector4);


	INLINE bool operator== (Scalar const& lhs, Scalar const& rhs) { return (float)lhs == (float)rhs; }
	INLINE bool operator== (Vector3 const& lhs, Vector3 const& rhs) { return DirectX::XMVector3Equal(lhs, rhs); }
	INLINE bool operator== (Vector4 const& lhs, Vector4 const& rhs) { return DirectX::XMVector4Equal(lhs, rhs); }

#undef CREATE_SIMD_FUNCTIONS

	INLINE float Sqrt(float s) { return Sqrt(Scalar(s)); }
	INLINE float Recip(float s) { return Recip(Scalar(s)); }
	INLINE float RecipSqrt(float s) { return RecipSqrt(Scalar(s)); }
	INLINE float Floor(float s) { return Floor(Scalar(s)); }
	INLINE float Ceiling(float s) { return Ceiling(Scalar(s)); }
	INLINE float Round(float s) { return Round(Scalar(s)); }
	INLINE float Abs(float s) { return s < 0.0f ? -s : s; }
	INLINE float Exp(float s) { return Exp(Scalar(s)); }
	INLINE float Pow(float b, float e) { return Pow(Scalar(b), Scalar(e)); }
	INLINE float Log(float s) { return Log(Scalar(s)); }
	INLINE float Sin(float s) { return Sin(Scalar(s)); }
	INLINE float Cos(float s) { return Cos(Scalar(s)); }
	INLINE float Tan(float s) { return Tan(Scalar(s)); }
	INLINE float ASin(float s) { return ASin(Scalar(s)); }
	INLINE float ACos(float s) { return ACos(Scalar(s)); }
	INLINE float ATan(float s) { return ATan(Scalar(s)); }
	INLINE float ATan2(float y, float x) { return ATan2(Scalar(y), Scalar(x)); }
	INLINE float Lerp(float a, float b, float t) { return a + (b - a) * t; }
	INLINE float Max(float a, float b) { return a > b ? a : b; }
	INLINE float Min(float a, float b) { return a < b ? a : b; }
	INLINE float Clamp(float v, float a, float b) { return Min(Max(v, a), b); }

	template<typename T>
	INLINE T Min(T a, T b) { return std::min(a, b); }

	template<typename T>
	INLINE T Max(T a, T b) { return std::max(a, b); }

	INLINE float GetMaxComponent(Vector3 const& vec) { return D_MATH::Max(vec.GetX(), D_MATH::Max(vec.GetY(), vec.GetZ())); }

	INLINE float GetMaxComponent(Vector4 const& vec) { return D_MATH::Max(vec.GetX(), D_MATH::Max(vec.GetY(), D_MATH::Max(vec.GetZ(), vec.GetW()))); }

	template<typename T>
	INLINE UINT Clamp(T v, T a, T b) { return Min(Max(v, a), b); }

	INLINE Scalar Length(Vector3 const& v) { return Scalar(DirectX::XMVector3Length(v)); }
	INLINE Scalar LengthSquare(Vector3 const& v) { return Scalar(DirectX::XMVector3LengthSq(v)); }
	INLINE Scalar LengthRecip(Vector3 const& v) { return Scalar(DirectX::XMVector3ReciprocalLength(v)); }
	INLINE Scalar Dot(Vector3 const& v1, Vector3 const& v2) { return Scalar(DirectX::XMVector3Dot(v1, v2)); }
	INLINE Scalar Dot(Vector4 const& v1, Vector4 const& v2) { return Scalar(DirectX::XMVector4Dot(v1, v2)); }
	INLINE Vector3 Cross(Vector3 const& v1, Vector3 const& v2) { return Vector3(DirectX::XMVector3Cross(v1, v2)); }
	INLINE Vector3 Normalize(Vector3 const& v) { return Vector3(DirectX::XMVector3Normalize(v)); }
	INLINE Vector4 Normalize(Vector4 const& v) { return Vector4(DirectX::XMVector4Normalize(v)); }

	INLINE Matrix3 Transpose(const Matrix3& mat) { return Matrix3(DirectX::XMMatrixTranspose(mat)); }
	INLINE Matrix3 InverseTranspose(const Matrix3& mat)
	{
		const Vector3 x = mat.GetX();
		const Vector3 y = mat.GetY();
		const Vector3 z = mat.GetZ();

		const Vector3 inv0 = Cross(y, z);
		const Vector3 inv1 = Cross(z, x);
		const Vector3 inv2 = Cross(x, y);
		const Scalar  rDet = Recip(Dot(z, inv2));

		// Return the adjoint / determinant
		return Matrix3(inv0, inv1, inv2) * rDet;
	}

	// inline Matrix3 Inverse( const Matrix3& mat ) { TBD }
	// inline Transform Inverse( const Transform& mat ) { TBD }

	// This specialized matrix invert assumes that the 3x3 matrix is orthogonal (and normalized).
	INLINE AffineTransform OrthoInvert(const AffineTransform& xform)
	{
		Matrix3 basis = Transpose(xform.GetBasis());
		return AffineTransform(basis, basis * -xform.GetTranslation());
	}

	INLINE OrthogonalTransform Invert(const OrthogonalTransform& xform) { return ~xform; }

	INLINE Matrix4 Transpose(const Matrix4& mat) { return Matrix4(DirectX::XMMatrixTranspose(mat)); }
	INLINE Matrix4 Invert(const Matrix4& mat) { return Matrix4(DirectX::XMMatrixInverse(nullptr, mat)); }
	

	INLINE Quaternion LookTowards(Vector3 const& dir, Vector3 const& up)
	{
		auto z = Normalize(dir);
		auto x = Normalize(Cross(up, dir));
		auto y = Cross(z, x);

		Matrix3 mat(x, y, z);

		return Quaternion(Matrix3(x, y, z));
	}

	INLINE Quaternion LookTowards(Vector3 const& dir) { return LookTowards(dir, Vector3::Up); }

	INLINE Quaternion LookAt(Vector3 const& eyePos, Vector3 const& target, Vector3 const& up)
	{
		return LookTowards(target - eyePos, up);
	}

	INLINE Quaternion LookAt(Vector3 const& eyePos, Vector3 const& target)
	{
		return LookTowards(target - eyePos);
	}

	INLINE Quaternion RotationOfVectors(Vector3 const& first, Vector3 const& second)
	{
		auto cross = D_MATH::Cross(first, second);
		float w = Sqrt(Pow((float)Length(first), 2.f) * Pow((float)Length(second), 2.f)) + Dot(first, second);
		return Quaternion(cross.GetX(), cross.GetY(), cross.GetZ(), w);
	}


	INLINE Matrix4 OrthoInvert(const Matrix4& xform)
	{
		Matrix3 basis = Transpose(xform.Get3x3());
		Vector3 translate = basis * -Vector3(xform.GetW());
		return Matrix4(basis, translate);
	}

	INLINE float Linear(float t, float b, float c, float d)
	{
		return c * (t / d) + b;
	}

	INLINE float ExpoEaseIn(float t, float b, float c, float d)
	{
		return (t == 0) ? b : c * pow(2, 10 * (t / d - 1)) + b;
	}

	INLINE float ExpoEaseOut(float t, float b, float c, float d)
	{
		return (t == d) ? b + c : c * pow(2, 1 - (10 * t / d)) + b;
	}

	INLINE float ExpoEaseInOut(float t, float b, float c, float d)
	{
		if (t == 0)
			return b;
		if (t == d)
			return b += c;
		if ((t /= d / 2) < 1)
			return c / 2 * pow(2, 10 * (t - 1)) + b;

		return c / 2 * pow(2, 1 - (10 * --t)) + b;
	}

	INLINE float CubicEaseIn(float t, float b, float c, float d)
	{
		return (t == 0) ? b : c * pow(3, 10 * (t / d - 1)) + b;
	}

	INLINE float CubicEaseOut(float t, float b, float c, float d)
	{
		return (t == d) ? b + c : c * pow(3, 1 - (10 * t / d)) + b;
	}

	INLINE float CubicEaseInOut(float t, float b, float c, float d)
	{
		if (t == 0)
			return b;
		if (t == d)
			return b += c;
		if ((t /= d / 2) < 1)
			return c / 2 * pow(3, 10 * (t - 1)) + b;

		return c / 2 * pow(3, 1 - (10 * --t)) + b;
	}

	INLINE float QuarticEaseIn(float t, float b, float c, float d)
	{
		return c * (t /= d) * t * t * t + b;
	}

	INLINE float QuarticEaseOut(float t, float b, float c, float d)
	{
		return -c * (1 - (t = t / d - 1) * t * t * t - 1) + b;
	}

	INLINE float QuarticEaseInOut(float t, float b, float c, float d)
	{
		t /= d / 2;
		if (t < 1)
			return c / 2 * t * t * t * t + b;
		t -= 2;
		return 1 - (-c / 2 * (t * t * t * t - 2)) + b;
	}

	INLINE float QuinticEaseIn(float t, float b, float c, float d)
	{
		return c * (t /= d) * t * t * t * t + b;
	}

	INLINE float QuinticEaseOut(float t, float b, float c, float d)
	{
		return c * (t = t / d - 1) * (1 - (t * t * t * t + 1)) + b;
	}

	INLINE float QuinticEaseInOut(float t, float b, float c, float d)
	{
		if ((t /= d / 2) < 1)
			return c / 2 * t * t * t * t * t + b;

		return 1 - (c / 2 * ((t -= 2) * (t * t * t * t) + 2)) + b;
	}

	INLINE float QuadraticEaseIn(float t, float b, float c, float d)
	{
		return c * (t /= d) * t + b;
	}

	INLINE float QuadraticEaseOut(float t, float b, float c, float d)
	{
		return 1 - (-c * (t /= d) * (t - 2)) + b;
	}

	INLINE float QuadraticEaseInOut(float t, float b, float c, float d)
	{
		if ((t /= d / 2) < 1)
			return ((c / 2) * (t * t)) + b;

		return 1 - (-c / 2 * (((--t) * (t - 2)) - 1)) + b;
	}

	INLINE float SineEaseIn(float t, float b, float c, float d)
	{
		return -c * cos(t / d * (DirectX::XM_PI / 2)) + c + b;
	}

	INLINE float SineEaseOut(float t, float b, float c, float d)
	{
		return c / 2 * cos(t / d * (DirectX::XM_PI / 2)) + b;
	}

	INLINE float SineEaseInOut(float t, float b, float c, float d)
	{
		if (t < 0.5f)
			return c /= d;

		return 1 - (-c / 2 * (cos(DirectX::XM_PI * t / d) - 1)) + b;
	}

	INLINE float CircularEaseIn(float t, float b, float c, float d)
	{
		return -c * (sqrt(1 - (t /= d) * t) - 1) + b;
	}

	INLINE float CircularEaseOut(float t, float b, float c, float d)
	{
		return 1 - (c * sqrt(1 - ((t = t / d - 1) * t))) + b;
	}

	INLINE float CircularEaseInOut(float t, float b, float c, float d)
	{
		if ((t /= d / 2) < 1)
			return -c / 2 * (sqrt(1 - t * t) - 1) + b;

		return 1 - (c / 2 * (sqrt(1 - t * (t -= 2)) + 1)) + b;
	}

	INLINE float BackEaseIn(float t, float b, float c, float d)
	{
		float s = 1.70158f;
		float postFix = t /= d;
		return c * (postFix)*t * ((s + 1) * t - s) + b;
	}

	INLINE float BackEaseOut(float t, float b, float c, float d)
	{
		float s = 1.70158f;
		return 1 - (c * ((t = t / d - 1) * t * ((s + 1) * t + s) + 1)) + b;
	}

	INLINE float BackEaseInOut(float t, float b, float c, float d)
	{
		float s = 1.70158f;
		if ((t /= d / 2) < 1)
			return c / 2 * (t * t * (((s *= (1.525f)) + 1) * t - s)) + b;

		float postFix = t -= 2;
		return 1 - (c / 2 * ((postFix)*t * (((s *= (1.525f)) + 1) * t + s) + 2)) + b;
	}

	INLINE float ElasticEaseIn(float t, float b, float c, float d)
	{
		if (t == 0)
			return b;
		if ((t /= d) == 1)
			return b + c;
		float p = d * .3f;
		float a = c;
		float s = p / 4;
		float postFix = a * pow(2, 10 * (t -= 1));
		return -(postFix * sin((t * d - s) * (2 * DirectX::XM_PI) / p)) + b;
	}

	INLINE float ElasticEaseOut(float t, float b, float c, float d)
	{
		if (t == 0)
			return b;
		if ((t /= d) == 1)
			return b + c;
		float p = d * .3f;
		float a = c;
		float s = p / 4;
		return 1 - (a * pow(2, -10 * t) * sin((t * d - s) * (2 * DirectX::XM_PI) / p) + c + b);
	}

	INLINE float ElasticEaseInOut(float t, float b, float c, float d)
	{
		if (t == 0)
			return b;
		if ((t /= d / 2) == 2)
			return b + c;
		float p = d * (.3f * 1.5f);
		float a = c;
		float s = p / 4;

		if (t < 1)
		{
			float postFix = a * pow(2, 10 * (t -= 1));
			return -.5f * (postFix * sin((t * d - s) * (2 * DirectX::XM_PI) / p)) + b;
		}
		float postFix = a * pow(2, -10 * (t -= 1));
		return 1 - (postFix * sin((t * d - s) * (2 * DirectX::XM_PI) / p) * .5f + c + b);
	}

	INLINE UINT CountTrailingZeros(UINT value)
	{
		// return 32 if value was 0
		unsigned long bitIndex;	// 0-based, where the LSB is 0 and MSB is 31
		return _BitScanForward(&bitIndex, value) ? bitIndex : 32;
	}

	INLINE UINT CountLeadingZeros(UINT value)
	{
		// return 32 if value is zero
		unsigned long bitIndex;
		if (!_BitScanReverse(&bitIndex, value)) bitIndex = -1;
		return 31 - bitIndex;
	}

	INLINE UINT CeilLogTwo(UINT arg)
	{
		// if Arg is 0, change it to 1 so that we return 0
		arg = arg ? arg : 1;
		return 32 - CountLeadingZeros(arg - 1);
	}

	INLINE UINT RoundUpToPowerOfTwo(UINT arg)
	{
		return 1 << CeilLogTwo(arg);
	}

}
#pragma warning(pop)