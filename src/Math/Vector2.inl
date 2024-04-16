#pragma once

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline Vector2& Vector2::operator+= (const Vector2& V)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V));
    const XMVECTOR X = XMVectorAdd(v1, v2);
    XMStoreFloat2(&mData, X);
    return *this;
}

inline Vector2& Vector2::operator-= (const Vector2& V)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V));
    const XMVECTOR X = XMVectorSubtract(v1, v2);
    XMStoreFloat2(&mData, X);
    return *this;
}

inline Vector2& Vector2::operator*= (const Vector2& V)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V));
    const XMVECTOR X = XMVectorMultiply(v1, v2);
    XMStoreFloat2(&mData, X);
    return *this;
}

inline Vector2& Vector2::operator*= (float S)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR X = XMVectorScale(v1, S);
    XMStoreFloat2(&mData, X);
    return *this;
}

inline Vector2& Vector2::operator/= (float S)
{
    using namespace DirectX;
    assert(S != 0.0f);
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR X = XMVectorScale(v1, 1.f / S);
    XMStoreFloat2(&mData, X);
    return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline Vector2 operator+ (const Vector2& V1, const Vector2& V2)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V1));
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V2));
    const XMVECTOR X = XMVectorAdd(v1, v2);
    Vector2 R;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
    return R;
}

inline Vector2 operator- (const Vector2& V1, const Vector2& V2)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V1));
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V2));
    const XMVECTOR X = XMVectorSubtract(v1, v2);
    Vector2 R;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
    return R;
}

inline Vector2 operator* (const Vector2& V1, const Vector2& V2)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V1));
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V2));
    const XMVECTOR X = XMVectorMultiply(v1, v2);
    Vector2 R;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
    return R;
}

inline Vector2 operator* (const Vector2& V, float S)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V));
    const XMVECTOR X = XMVectorScale(v1, S);
    Vector2 R;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
    return R;
}

inline Vector2 operator/ (const Vector2& V1, const Vector2& V2)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V1));
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V2));
    const XMVECTOR X = XMVectorDivide(v1, v2);
    Vector2 R;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
    return R;
}

inline Vector2 operator/ (const Vector2& V, float S)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V));
    const XMVECTOR X = XMVectorScale(v1, 1.f / S);
    Vector2 R;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
    return R;
}

inline Vector2 operator* (float S, const Vector2& V)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V));
    const XMVECTOR X = XMVectorScale(v1, S);
    Vector2 R;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
    return R;
}

//------------------------------------------------------------------------------
// Vector operations
//------------------------------------------------------------------------------

inline bool Vector2::InBounds(const Vector2& Bounds) const
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(Bounds));
    return XMVector2InBounds(v1, v2);
}

inline float Vector2::Length() const
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR X = XMVector2Length(v1);
    return XMVectorGetX(X);
}

inline float Vector2::LengthSquared() const
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR X = XMVector2LengthSq(v1);
    return XMVectorGetX(X);
}

inline float Vector2::Dot(const Vector2& V) const
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V));
    const XMVECTOR X = XMVector2Dot(v1, v2);
    return XMVectorGetX(X);
}

inline void Vector2::Cross(const Vector2& V, Vector2& result) const
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V));
    const XMVECTOR R = XMVector2Cross(v1, v2);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), R);
}

inline Vector2 Vector2::Cross(const Vector2& V) const
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(V));
    const XMVECTOR R = XMVector2Cross(v1, v2);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), R);
    return result;
}

inline Vector2 Vector2::Normal() const
{
    using namespace DirectX;
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR X = XMVector2Normalize(v1);
    Vector2 result;
    XMStoreFloat2(&result.mData, X);
}

inline void Vector2::Normalize()
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR X = XMVector2Normalize(v1);
    XMStoreFloat2(&mData, X);
}

inline void Vector2::Normalize(Vector2& result) const
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR X = XMVector2Normalize(v1);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline void Vector2::Clamp(const Vector2& vmin, const Vector2& vmax)
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(vmin));
    const XMVECTOR v3 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(vmax));
    const XMVECTOR X = XMVectorClamp(v1, v2, v3);
    XMStoreFloat2(&mData, X);
}

inline void Vector2::Clamp(const Vector2& vmin, const Vector2& vmax, Vector2& result) const
{
    using namespace DirectX;
    const XMVECTOR v1 = XMLoadFloat2(&mData);
    const XMVECTOR v2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(vmin));
    const XMVECTOR v3 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(vmax));
    const XMVECTOR X = XMVectorClamp(v1, v2, v3);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

//------------------------------------------------------------------------------
// Static functions
//------------------------------------------------------------------------------

inline float Vector2::Distance(const Vector2& v1, const Vector2& v2)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR V = XMVectorSubtract(x2, x1);
    const XMVECTOR X = XMVector2Length(V);
    return XMVectorGetX(X);
}

inline float Vector2::DistanceSquared(const Vector2& v1, const Vector2& v2)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR V = XMVectorSubtract(x2, x1);
    const XMVECTOR X = XMVector2LengthSq(V);
    return XMVectorGetX(X);
}

inline void Vector2::Min(const Vector2& v1, const Vector2& v2, Vector2& result)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR X = XMVectorMin(x1, x2);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline Vector2 Vector2::Min(const Vector2& v1, const Vector2& v2)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR X = XMVectorMin(x1, x2);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
    return result;
}

inline void Vector2::Max(const Vector2& v1, const Vector2& v2, Vector2& result)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR X = XMVectorMax(x1, x2);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline Vector2 Vector2::Max(const Vector2& v1, const Vector2& v2)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR X = XMVectorMax(x1, x2);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
    return result;
}

inline void Vector2::Lerp(const Vector2& v1, const Vector2& v2, float t, Vector2& result)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR X = XMVectorLerp(x1, x2, t);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline Vector2 Vector2::Lerp(const Vector2& v1, const Vector2& v2, float t)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR X = XMVectorLerp(x1, x2, t);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
    return result;
}

inline void Vector2::SmoothStep(const Vector2& v1, const Vector2& v2, float t, Vector2& result)
{
    using namespace DirectX;
    t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
    t = t * t * (3.f - 2.f * t);
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR X = XMVectorLerp(x1, x2, t);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline Vector2 Vector2::SmoothStep(const Vector2& v1, const Vector2& v2, float t)
{
    using namespace DirectX;
    t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
    t = t * t * (3.f - 2.f * t);
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR X = XMVectorLerp(x1, x2, t);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
    return result;
}

inline void Vector2::Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g, Vector2& result)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR x3 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v3));
    const XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline Vector2 Vector2::Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR x3 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v3));
    const XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
    return result;
}

inline void Vector2::CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t, Vector2& result)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR x3 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v3));
    const XMVECTOR x4 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v4));
    const XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline Vector2 Vector2::CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR x3 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v3));
    const XMVECTOR x4 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v4));
    const XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
    return result;
}

inline void Vector2::Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t, Vector2& result)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(t1));
    const XMVECTOR x3 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR x4 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(t2));
    const XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline Vector2 Vector2::Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t)
{
    using namespace DirectX;
    const XMVECTOR x1 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v1));
    const XMVECTOR x2 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(t1));
    const XMVECTOR x3 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(v2));
    const XMVECTOR x4 = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(t2));
    const XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
    return result;
}

inline void Vector2::Reflect(const Vector2& ivec, const Vector2& nvec, Vector2& result)
{
    using namespace DirectX;
    const XMVECTOR i = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(ivec));
    const XMVECTOR n = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(nvec));
    const XMVECTOR X = XMVector2Reflect(i, n);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline Vector2 Vector2::Reflect(const Vector2& ivec, const Vector2& nvec)
{
    using namespace DirectX;
    const XMVECTOR i = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(ivec));
    const XMVECTOR n = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(nvec));
    const XMVECTOR X = XMVector2Reflect(i, n);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
    return result;
}

inline void Vector2::Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex, Vector2& result)
{
    using namespace DirectX;
    const XMVECTOR i = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(ivec));
    const XMVECTOR n = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(nvec));
    const XMVECTOR X = XMVector2Refract(i, n, refractionIndex);
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
}

inline Vector2 Vector2::Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex)
{
    using namespace DirectX;
    const XMVECTOR i = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(ivec));
    const XMVECTOR n = XMLoadFloat2(&static_cast<XMFLOAT2 const&>(nvec));
    const XMVECTOR X = XMVector2Refract(i, n, refractionIndex);

    Vector2 result;
    XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
    return result;
}

inline bool Vector2::Equals(Vector2 const& other) const
{
    return mData.x == other.mData.x &&
        mData.y == other.mData.y;
}

inline bool Vector2::NearEquals(Vector2 const& other, float epsilon) const
{
    return std::abs(mData.x - other.mData.x) < epsilon &&
        std::abs(mData.y - other.mData.y) < epsilon;
}
