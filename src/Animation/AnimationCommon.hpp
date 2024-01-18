#pragma once

#include <Core/Containers/Map.hpp>
#include <Core/Serialization/Json.hpp>
#include <Math/VectorMath.hpp>
#include <Math/Color.hpp>

#include <rttr/rttr_enable.h>
#include <rttr/registration_friend.h>

#include <optional>

#include "AnimationCommon.generated.hpp"

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

namespace Darius::Animation
{
    struct DStruct(Serialize) Keyframe
    {

        GENERATED_BODY();

    public:
        DField(Serialize)
        float               Time = 0.f;

    private:
        DField(Serialize)
        D_MATH::Vector4     Value = D_MATH::Vector4::Zero;

        DField(Serialize)
        D_MATH::Vector4     InTangent = D_MATH::Vector4::Zero;

        DField(Serialize)
        D_MATH::Vector4     OutTangent = D_MATH::Vector4::Zero;

    public:
        template<typename T>
        INLINE T& GetValue()
        {
            D_ASSERT_M(false, "Unsupported type for keyframe");
            return *reinterpret_cast<T*>(nullptr);
        }

        template<typename T>
        INLINE T const& GetValue() const
        {
            D_ASSERT_M(false, "Unsupported type for keyframe");
            return *reinterpret_cast<T const*>(nullptr);
        }

        template<typename T>
        INLINE T& GetInTangent()
        {
            D_ASSERT_M(false, "Unsupported type for keyframe");
            return *reinterpret_cast<T*>(nullptr);
        }

        template<typename T>
        INLINE T const& GetInTangent() const
        {
            D_ASSERT_M(false, "Unsupported type for keyframe");
            return *reinterpret_cast<T const*>(nullptr);
        }

        template<typename T>
        INLINE T& GetOutTangent()
        {
            D_ASSERT_M(false, "Unsupported type for keyframe");
            return *reinterpret_cast<T*>(nullptr);
        }

        template<typename T>
        INLINE T const& GetOutTangent() const
        {
            D_ASSERT_M(false, "Unsupported type for keyframe");
            return *reinterpret_cast<T const*>(nullptr);
        }

        // Value Getters
        template<>
        INLINE bool& GetValue<bool>() { return *reinterpret_cast<bool*>(&Value); }
        template<>
        INLINE bool const& GetValue<bool const>() const { return *reinterpret_cast<bool const*>(&Value); }
        template<>
        INLINE int& GetValue<int>() { return *reinterpret_cast<int*>(&Value); }
        template<>
        INLINE int const& GetValue<int const>() const { return *reinterpret_cast<int const*>(&Value); }
        template<>
        INLINE UINT& GetValue<UINT>() { return *reinterpret_cast<UINT*>(&Value); }
        template<>
        INLINE UINT const& GetValue<UINT const>() const { return *reinterpret_cast<UINT const*>(&Value); }
        template<>
        INLINE short& GetValue<short>() { return *reinterpret_cast<short*>(&Value); }
        template<>
        INLINE short const& GetValue<short const>() const { return *reinterpret_cast<short const*>(&Value); }
        template<>
        INLINE USHORT& GetValue<USHORT>() { return *reinterpret_cast<USHORT*>(&Value); }
        template<>
        INLINE USHORT const& GetValue<USHORT const>() const { return *reinterpret_cast<USHORT const*>(&Value); }
        template<>
        INLINE long& GetValue<long>() { return *reinterpret_cast<long*>(&Value); }
        template<>
        INLINE long const& GetValue<long const>() const { return *reinterpret_cast<long const*>(&Value); }
        template<>
        INLINE ULONG& GetValue<ULONG>() { return *reinterpret_cast<ULONG*>(&Value); }
        template<>
        INLINE ULONG const& GetValue<ULONG const>() const { return *reinterpret_cast<ULONG const*>(&Value); }
        template<>
        INLINE D_MATH::Vector2& GetValue<D_MATH::Vector2>() { return *reinterpret_cast<D_MATH::Vector2*>(&Value); }
        template<>
        INLINE D_MATH::Vector2 const& GetValue<D_MATH::Vector2 const>() const { return *reinterpret_cast<D_MATH::Vector2 const*>(&Value); }
        template<>
        INLINE D_MATH::Vector3& GetValue<D_MATH::Vector3>() { return *reinterpret_cast<D_MATH::Vector3*>(&Value); }
        template<>
        INLINE D_MATH::Vector3 const& GetValue<D_MATH::Vector3 const>() const { return *reinterpret_cast<D_MATH::Vector3 const*>(&Value); }
        template<>
        INLINE D_MATH::Vector4& GetValue<D_MATH::Vector4>() { return *reinterpret_cast<D_MATH::Vector4*>(&Value); }
        template<>
        INLINE D_MATH::Vector4 const& GetValue<D_MATH::Vector4 const>() const { return *reinterpret_cast<D_MATH::Vector4 const*>(&Value); }
        template<>
        INLINE D_MATH::Color& GetValue<D_MATH::Color>() { return *reinterpret_cast<D_MATH::Color*>(&Value); }
        template<>
        INLINE D_MATH::Color const& GetValue<D_MATH::Color const>() const { return *reinterpret_cast<D_MATH::Color const*>(&Value); }
        template<>
        INLINE D_MATH::Quaternion& GetValue<D_MATH::Quaternion>() { return *reinterpret_cast<D_MATH::Quaternion*>(&Value); }
        template<>
        INLINE D_MATH::Quaternion const& GetValue<D_MATH::Quaternion const>() const { return *reinterpret_cast<D_MATH::Quaternion const*>(&Value); }

        // In Tangent Getters
        template<>
        INLINE bool& GetInTangent<bool>() { return *reinterpret_cast<bool*>(&InTangent); }
        template<>
        INLINE bool const& GetInTangent<bool const>() const { return *reinterpret_cast<bool const*>(&InTangent); }
        template<>
        INLINE int& GetInTangent<int>() { return *reinterpret_cast<int*>(&InTangent); }
        template<>
        INLINE int const& GetInTangent<int const>() const { return *reinterpret_cast<int const*>(&InTangent); }
        template<>
        INLINE UINT& GetInTangent<UINT>() { return *reinterpret_cast<UINT*>(&InTangent); }
        template<>
        INLINE UINT const& GetInTangent<UINT const>() const { return *reinterpret_cast<UINT const*>(&InTangent); }
        template<>
        INLINE short& GetInTangent<short>() { return *reinterpret_cast<short*>(&InTangent); }
        template<>
        INLINE short const& GetInTangent<short const>() const { return *reinterpret_cast<short const*>(&InTangent); }
        template<>
        INLINE USHORT& GetInTangent<USHORT>() { return *reinterpret_cast<USHORT*>(&InTangent); }
        template<>
        INLINE USHORT const& GetInTangent<USHORT const>() const { return *reinterpret_cast<USHORT const*>(&InTangent); }
        template<>
        INLINE long& GetInTangent<long>() { return *reinterpret_cast<long*>(&InTangent); }
        template<>
        INLINE long const& GetInTangent<long const>() const { return *reinterpret_cast<long const*>(&InTangent); }
        template<>
        INLINE ULONG& GetInTangent<ULONG>() { return *reinterpret_cast<ULONG*>(&InTangent); }
        template<>
        INLINE ULONG const& GetInTangent<ULONG const>() const { return *reinterpret_cast<ULONG const*>(&InTangent); }
        template<>
        INLINE D_MATH::Vector2& GetInTangent<D_MATH::Vector2>() { return *reinterpret_cast<D_MATH::Vector2*>(&InTangent); }
        template<>
        INLINE D_MATH::Vector2 const& GetInTangent<D_MATH::Vector2 const>() const { return *reinterpret_cast<D_MATH::Vector2 const*>(&InTangent); }
        template<>
        INLINE D_MATH::Vector3& GetInTangent<D_MATH::Vector3>() { return *reinterpret_cast<D_MATH::Vector3*>(&InTangent); }
        template<>
        INLINE D_MATH::Vector3 const& GetInTangent<D_MATH::Vector3 const>() const { return *reinterpret_cast<D_MATH::Vector3 const*>(&InTangent); }
        template<>
        INLINE D_MATH::Vector4& GetInTangent<D_MATH::Vector4>() { return *reinterpret_cast<D_MATH::Vector4*>(&InTangent); }
        template<>
        INLINE D_MATH::Vector4 const& GetInTangent<D_MATH::Vector4 const>() const { return *reinterpret_cast<D_MATH::Vector4 const*>(&InTangent); }
        template<>
        INLINE D_MATH::Color& GetInTangent<D_MATH::Color>() { return *reinterpret_cast<D_MATH::Color*>(&InTangent); }
        template<>
        INLINE D_MATH::Color const& GetInTangent<D_MATH::Color const>() const { return *reinterpret_cast<D_MATH::Color const*>(&InTangent); }
        template<>
        INLINE D_MATH::Quaternion& GetInTangent<D_MATH::Quaternion>() { return *reinterpret_cast<D_MATH::Quaternion*>(&InTangent); }
        template<>
        INLINE D_MATH::Quaternion const& GetInTangent<D_MATH::Quaternion const>() const { return *reinterpret_cast<D_MATH::Quaternion const*>(&InTangent); }

        // Out Tangent Getters
        template<>
        INLINE bool& GetOutTangent<bool>() { return *reinterpret_cast<bool*>(&OutTangent); }
        template<>
        INLINE bool const& GetOutTangent<bool const>() const { return *reinterpret_cast<bool const*>(&OutTangent); }
        template<>
        INLINE int& GetOutTangent<int>() { return *reinterpret_cast<int*>(&OutTangent); }
        template<>
        INLINE int const& GetOutTangent<int const>() const { return *reinterpret_cast<int const*>(&OutTangent); }
        template<>
        INLINE UINT& GetOutTangent<UINT>() { return *reinterpret_cast<UINT*>(&OutTangent); }
        template<>
        INLINE UINT const& GetOutTangent<UINT const>() const { return *reinterpret_cast<UINT const*>(&OutTangent); }
        template<>
        INLINE short& GetOutTangent<short>() { return *reinterpret_cast<short*>(&OutTangent); }
        template<>
        INLINE short const& GetOutTangent<short const>() const { return *reinterpret_cast<short const*>(&OutTangent); }
        template<>
        INLINE USHORT& GetOutTangent<USHORT>() { return *reinterpret_cast<USHORT*>(&OutTangent); }
        template<>
        INLINE USHORT const& GetOutTangent<USHORT const>() const { return *reinterpret_cast<USHORT const*>(&OutTangent); }
        template<>
        INLINE long& GetOutTangent<long>() { return *reinterpret_cast<long*>(&OutTangent); }
        template<>
        INLINE long const& GetOutTangent<long const>() const { return *reinterpret_cast<long const*>(&OutTangent); }
        template<>
        INLINE ULONG& GetOutTangent<ULONG>() { return *reinterpret_cast<ULONG*>(&OutTangent); }
        template<>
        INLINE ULONG const& GetOutTangent<ULONG const>() const { return *reinterpret_cast<ULONG const*>(&OutTangent); }
        template<>
        INLINE D_MATH::Vector2& GetOutTangent<D_MATH::Vector2>() { return *reinterpret_cast<D_MATH::Vector2*>(&OutTangent); }
        template<>
        INLINE D_MATH::Vector2 const& GetOutTangent<D_MATH::Vector2 const>() const { return *reinterpret_cast<D_MATH::Vector2 const*>(&OutTangent); }
        template<>
        INLINE D_MATH::Vector3& GetOutTangent<D_MATH::Vector3>() { return *reinterpret_cast<D_MATH::Vector3*>(&OutTangent); }
        template<>
        INLINE D_MATH::Vector3 const& GetOutTangent<D_MATH::Vector3 const>() const { return *reinterpret_cast<D_MATH::Vector3 const*>(&OutTangent); }
        template<>
        INLINE D_MATH::Vector4& GetOutTangent<D_MATH::Vector4>() { return *reinterpret_cast<D_MATH::Vector4*>(&OutTangent); }
        template<>
        INLINE D_MATH::Vector4 const& GetOutTangent<D_MATH::Vector4 const>() const { return *reinterpret_cast<D_MATH::Vector4 const*>(&OutTangent); }
        template<>
        INLINE D_MATH::Color& GetOutTangent<D_MATH::Color>() { return *reinterpret_cast<D_MATH::Color*>(&OutTangent); }
        template<>
        INLINE D_MATH::Color const& GetOutTangent<D_MATH::Color const>() const { return *reinterpret_cast<D_MATH::Color const*>(&OutTangent); }
        template<>
        INLINE D_MATH::Quaternion& GetOutTangent<D_MATH::Quaternion>() { return *reinterpret_cast<D_MATH::Quaternion*>(&OutTangent); }
        template<>
        INLINE D_MATH::Quaternion const& GetOutTangent<D_MATH::Quaternion const>() const { return *reinterpret_cast<D_MATH::Quaternion const*>(&OutTangent); }
    };

    enum class DEnum(Serialize) InterpolationMode
    {
        Step,
        Linear,
        Slerp,
        CatmullRomSpline,
        HermiteSpline
    };

    enum class DEnum(Serialize) KeyframeDataType
    {
        Bool,
        Int,
        Uint,
        Short,
        Ushort,
        Long,
        Ulong,
        Vector2,
        Vector3,
        Vector4,
        Color,
        Quaternion
    };
    
#pragma region Interpolations
    template<typename T>
    T Interpolate(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt)
    {
        switch (mode)
        {
        case InterpolationMode::Step:
            return b.GetValue<T>();

        case InterpolationMode::Linear:
            return (T)D_MATH::Lerp(b.GetValue<T>(), c.GetValue<T>(), t);

        case InterpolationMode::Slerp:
        {
            D_ASSERT_M(false, "Illegal interpolation slerp for the provided type");
            return T();
        }

        case InterpolationMode::CatmullRomSpline:
        {
            // https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval_with_matched_derivatives_at_endpoints
            // a = p[n-1], b = p[n], c = p[n+1], d = p[n+2]
            T const& aVal = a.GetValue<T>();
            T const& bVal = b.GetValue<T>();
            T const& cVal = c.GetValue<T>();
            T const& dVal = d.GetValue<T>();
            auto i = -aVal + 3.f * bVal - 3.f * cVal + dVal;
            auto j = 2.f * aVal - 5.f * bVal + 4.f * cVal - dVal;
            auto k = -aVal + cVal;
            return (T)(0.5f * ((i * t + j) * t + k) * t + bVal);
        }

        case InterpolationMode::HermiteSpline:
        {
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
            const float t2 = t * t;
            const float t3 = t2 * t;
            return (T)((2.f * t3 - 3.f * t2 + 1.f) * b.GetValue<T>()
                + (t3 - 2.f * t2 + t) * b.GetOutTangent<T>() * dt
                + (-2.f * t3 + 3.f * t2) * c.GetValue<T>()
                + (t3 - t2) * c.GetInTangent<T>() * dt);
        }

        default:
            D_ASSERT_M(false, "Unknown interpolation mode");
            return b.GetValue<T>();
        }
    }

    template<>
    INLINE int Interpolate<int>(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt)
    {
        using T = int;
        switch (mode)
        {
        case InterpolationMode::Step:
            return b.GetValue<T>();

        case InterpolationMode::Linear:
            return (T)D_MATH::Lerp((float)b.GetValue<T>(), (float)c.GetValue<T>(), t);

        case InterpolationMode::Slerp:
        {
            D_ASSERT_M(false, "Illegal interpolation slerp for the provided type");
            return T();
        }

        case InterpolationMode::CatmullRomSpline:
        {
            // https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval_with_matched_derivatives_at_endpoints
            // a = p[n-1], b = p[n], c = p[n+1], d = p[n+2]
            T const& aVal = a.GetValue<T>();
            T const& bVal = b.GetValue<T>();
            T const& cVal = c.GetValue<T>();
            T const& dVal = d.GetValue<T>();
            auto i = -aVal + 3.f * bVal - 3.f * cVal + dVal;
            auto j = 2.f * aVal - 5.f * bVal + 4.f * cVal - dVal;
            auto k = -aVal + cVal;
            return (T)(0.5f * ((i * t + j) * t + k) * t + bVal);
        }

        case InterpolationMode::HermiteSpline:
        {
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
            const float t2 = t * t;
            const float t3 = t2 * t;
            return (T)((2.f * t3 - 3.f * t2 + 1.f) * b.GetValue<T>()
                + (t3 - 2.f * t2 + t) * b.GetOutTangent<T>() * dt
                + (-2.f * t3 + 3.f * t2) * c.GetValue<T>()
                + (t3 - t2) * c.GetInTangent<T>() * dt);
        }

        default:
            D_ASSERT_M(false, "Unknown interpolation mode");
            return b.GetValue<T>();
        }
    }

    template<>
    INLINE UINT Interpolate<UINT>(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt)
    {
        using T = UINT;
        switch (mode)
        {
        case InterpolationMode::Step:
            return b.GetValue<T>();

        case InterpolationMode::Linear:
            return (T)D_MATH::Lerp((float)b.GetValue<T>(), (float)c.GetValue<T>(), t);

        case InterpolationMode::Slerp:
        {
            D_ASSERT_M(false, "Illegal interpolation slerp for the provided type");
            return T();
        }

        case InterpolationMode::CatmullRomSpline:
        {
            // https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval_with_matched_derivatives_at_endpoints
            // a = p[n-1], b = p[n], c = p[n+1], d = p[n+2]
            T const& aVal = a.GetValue<T>();
            T const& bVal = b.GetValue<T>();
            T const& cVal = c.GetValue<T>();
            T const& dVal = d.GetValue<T>();
            auto i = 3.f * bVal - 3.f * cVal + dVal - aVal;
            auto j = 2.f * aVal - 5.f * bVal + 4.f * cVal - dVal;
            auto k = cVal - aVal;
            return (T)(0.5f * ((i * t + j) * t + k) * t + bVal);
        }

        case InterpolationMode::HermiteSpline:
        {
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
            const float t2 = t * t;
            const float t3 = t2 * t;
            return (T)((2.f * t3 - 3.f * t2 + 1.f) * b.GetValue<T>()
                + (t3 - 2.f * t2 + t) * b.GetOutTangent<T>() * dt
                + (-2.f * t3 + 3.f * t2) * c.GetValue<T>()
                + (t3 - t2) * c.GetInTangent<T>() * dt);
        }

        default:
            D_ASSERT_M(false, "Unknown interpolation mode");
            return b.GetValue<T>();
        }
    }

    template<>
    INLINE short Interpolate<short>(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt)
    {
        using T = short;
        switch (mode)
        {
        case InterpolationMode::Step:
            return b.GetValue<T>();

        case InterpolationMode::Linear:
            return (T)D_MATH::Lerp((float)b.GetValue<T>(), (float)c.GetValue<T>(), t);

        case InterpolationMode::Slerp:
        {
            D_ASSERT_M(false, "Illegal interpolation slerp for the provided type");
            return T();
        }

        case InterpolationMode::CatmullRomSpline:
        {
            // https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval_with_matched_derivatives_at_endpoints
            // a = p[n-1], b = p[n], c = p[n+1], d = p[n+2]
            T const& aVal = a.GetValue<T>();
            T const& bVal = b.GetValue<T>();
            T const& cVal = c.GetValue<T>();
            T const& dVal = d.GetValue<T>();
            auto i = -aVal + 3.f * bVal - 3.f * cVal + dVal;
            auto j = 2.f * aVal - 5.f * bVal + 4.f * cVal - dVal;
            auto k = -aVal + cVal;
            return (T)(0.5f * ((i * t + j) * t + k) * t + bVal);
        }

        case InterpolationMode::HermiteSpline:
        {
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
            const float t2 = t * t;
            const float t3 = t2 * t;
            return (T)((2.f * t3 - 3.f * t2 + 1.f) * b.GetValue<T>()
                + (t3 - 2.f * t2 + t) * b.GetOutTangent<T>() * dt
                + (-2.f * t3 + 3.f * t2) * c.GetValue<T>()
                + (t3 - t2) * c.GetInTangent<T>() * dt);
        }

        default:
            D_ASSERT_M(false, "Unknown interpolation mode");
            return b.GetValue<T>();
        }
    }

    template<>
    INLINE USHORT Interpolate<USHORT>(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt)
    {
        using T = USHORT;
        switch (mode)
        {
        case InterpolationMode::Step:
            return b.GetValue<T>();

        case InterpolationMode::Linear:
            return (T)D_MATH::Lerp((float)b.GetValue<T>(), (float)c.GetValue<T>(), t);

        case InterpolationMode::Slerp:
        {
            D_ASSERT_M(false, "Illegal interpolation slerp for the provided type");
            return T();
        }

        case InterpolationMode::CatmullRomSpline:
        {
            // https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval_with_matched_derivatives_at_endpoints
            // a = p[n-1], b = p[n], c = p[n+1], d = p[n+2]
            T const& aVal = a.GetValue<T>();
            T const& bVal = b.GetValue<T>();
            T const& cVal = c.GetValue<T>();
            T const& dVal = d.GetValue<T>();
            auto i = -aVal + 3.f * bVal - 3.f * cVal + dVal;
            auto j = 2.f * aVal - 5.f * bVal + 4.f * cVal - dVal;
            auto k = -aVal + cVal;
            return (T)(0.5f * ((i * t + j) * t + k) * t + bVal);
        }

        case InterpolationMode::HermiteSpline:
        {
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
            const float t2 = t * t;
            const float t3 = t2 * t;
            return (T)((2.f * t3 - 3.f * t2 + 1.f) * b.GetValue<T>()
                + (t3 - 2.f * t2 + t) * b.GetOutTangent<T>() * dt
                + (-2.f * t3 + 3.f * t2) * c.GetValue<T>()
                + (t3 - t2) * c.GetInTangent<T>() * dt);
        }

        default:
            D_ASSERT_M(false, "Unknown interpolation mode");
            return b.GetValue<T>();
        }
    }

    template<>
    INLINE long Interpolate<long>(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt)
    {
        using T = long;
        switch (mode)
        {
        case InterpolationMode::Step:
            return b.GetValue<T>();

        case InterpolationMode::Linear:
            return (T)D_MATH::Lerp((float)b.GetValue<T>(), (float)c.GetValue<T>(), t);

        case InterpolationMode::Slerp:
        {
            D_ASSERT_M(false, "Illegal interpolation slerp for the provided type");
            return T();
        }

        case InterpolationMode::CatmullRomSpline:
        {
            // https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval_with_matched_derivatives_at_endpoints
            // a = p[n-1], b = p[n], c = p[n+1], d = p[n+2]
            T const& aVal = a.GetValue<T>();
            T const& bVal = b.GetValue<T>();
            T const& cVal = c.GetValue<T>();
            T const& dVal = d.GetValue<T>();
            auto i = -aVal + 3.f * bVal - 3.f * cVal + dVal;
            auto j = 2.f * aVal - 5.f * bVal + 4.f * cVal - dVal;
            auto k = -aVal + cVal;
            return (T)(0.5f * ((i * t + j) * t + k) * t + bVal);
        }

        case InterpolationMode::HermiteSpline:
        {
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
            const float t2 = t * t;
            const float t3 = t2 * t;
            return (T)((2.f * t3 - 3.f * t2 + 1.f) * b.GetValue<T>()
                + (t3 - 2.f * t2 + t) * b.GetOutTangent<T>() * dt
                + (-2.f * t3 + 3.f * t2) * c.GetValue<T>()
                + (t3 - t2) * c.GetInTangent<T>() * dt);
        }

        default:
            D_ASSERT_M(false, "Unknown interpolation mode");
            return b.GetValue<T>();
        }
    }

    template<>
    INLINE ULONG Interpolate<ULONG>(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt)
    {
        using T = ULONG;
        switch (mode)
        {
        case InterpolationMode::Step:
            return b.GetValue<T>();

        case InterpolationMode::Linear:
            return (T)D_MATH::Lerp((float)b.GetValue<T>(), (float)c.GetValue<T>(), t);

        case InterpolationMode::Slerp:
        {
            D_ASSERT_M(false, "Illegal interpolation slerp for the provided type");
            return T();
        }

        case InterpolationMode::CatmullRomSpline:
        {
            // https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval_with_matched_derivatives_at_endpoints
            // a = p[n-1], b = p[n], c = p[n+1], d = p[n+2]
            T const& aVal = a.GetValue<T>();
            T const& bVal = b.GetValue<T>();
            T const& cVal = c.GetValue<T>();
            T const& dVal = d.GetValue<T>();
            auto i = + 3.f * bVal - 3.f * cVal + dVal - aVal;
            auto j = 2.f * aVal - 5.f * bVal + 4.f * cVal - dVal;
            auto k = cVal - aVal;
            return (T)(0.5f * ((i * t + j) * t + k) * t + bVal);
        }

        case InterpolationMode::HermiteSpline:
        {
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
            const float t2 = t * t;
            const float t3 = t2 * t;
            return (T)((2.f * t3 - 3.f * t2 + 1.f) * b.GetValue<T>()
                + (t3 - 2.f * t2 + t) * b.GetOutTangent<T>() * dt
                + (-2.f * t3 + 3.f * t2) * c.GetValue<T>()
                + (t3 - t2) * c.GetInTangent<T>() * dt);
        }

        default:
            D_ASSERT_M(false, "Unknown interpolation mode");
            return b.GetValue<T>();
        }
    }

    template<>
    INLINE D_MATH::Quaternion Interpolate<D_MATH::Quaternion>(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt)
    {
        using T = D_MATH::Quaternion;

        switch (mode)
        {
        case InterpolationMode::Step:
            return b.GetValue<T>();

        case InterpolationMode::Linear:
            return D_MATH::Lerp(b.GetValue<T>(), c.GetValue<T>(), t);

        case InterpolationMode::Slerp:
        {
            D_MATH::Quaternion const& qb = b.GetValue<D_MATH::Quaternion>();
            D_MATH::Quaternion const& qc = c.GetValue<D_MATH::Quaternion>();
            return D_MATH::Slerp(qb, qc, t);
        }

        case InterpolationMode::CatmullRomSpline:
        {
            // https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval_with_matched_derivatives_at_endpoints
            // a = p[n-1], b = p[n], c = p[n+1], d = p[n+2]
            T const& aVal = a.GetValue<T>();
            T const& bVal = b.GetValue<T>();
            T const& cVal = c.GetValue<T>();
            T const& dVal = d.GetValue<T>();
            T i = -aVal + 3.f * bVal - 3.f * cVal + dVal;
            T j = 2.f * aVal - 5.f * bVal + 4.f * cVal - dVal;
            T k = -aVal + cVal;
            return 0.5f * ((i * t + j) * t + k) * t + bVal;
        }

        case InterpolationMode::HermiteSpline:
        {
            // https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
            const float t2 = t * t;
            const float t3 = t2 * t;
            return (2.f * t3 - 3.f * t2 + 1.f) * b.GetValue<T>()
                + (t3 - 2.f * t2 + t) * b.GetOutTangent<T>() * dt
                + (-2.f * t3 + 3.f * t2) * c.GetValue<T>()
                + (t3 - t2) * c.GetInTangent<T>() * dt;
        }

        default:
            D_ASSERT_M(false, "Unknown interpolation mode");
            return b.GetValue<T>();
        }
    }

#define StepOnlyInterpolationFunction(type) \
    template<> \
    INLINE type Interpolate<type>(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt) \
    { \
        switch (mode) \
        { \
        case InterpolationMode::Step: \
            return b.GetValue<type>(); \
        case InterpolationMode::Linear: \
        case InterpolationMode::Slerp: \
        case InterpolationMode::CatmullRomSpline: \
        case InterpolationMode::HermiteSpline: \
        default: \
            D_ASSERT_M(false, "Illegal interpolation slerp for the provided type"); \
            return b.GetValue<type>(); \
        } \
    } \

    StepOnlyInterpolationFunction(bool);

#undef StepOnlyInterpolationFunction

#pragma endregion
    
    // Represents a curve on a scalar or vector value
    class DClass(Serialize) Track
    {
        GENERATED_BODY();
    public:
        Track();
        Track(InterpolationMode mode, KeyframeDataType dataType);
        virtual ~Track() = default;

        template<typename T>
        std::optional<T>                        Evaluate(float time, bool extrapolateLastValues = false) const;

        NODISCARD D_CONTAINERS::DVector<Keyframe>& GetKeyframes() { return mKeyframes; }
        Keyframe*                               AddKeyframe(Keyframe const& keyframe, int index = -1);

        // The pointer might invalidate upon change of the number of keyframes, so do not keep a reference
        NODISCARD Keyframe const*               FindKeyframeByTime(float time) const;
        // The pointer might invalidate upon change of the number of keyframes, so do not keep a reference
        NODISCARD Keyframe*                     FindKeyframeByTime(float time);
        // The pointer might invalidate upon change of the number of keyframes, so do not keep a reference
        NODISCARD Keyframe*                     FindOrCreateKeyframeByTime(float time, bool* created = nullptr);


        NODISCARD INLINE InterpolationMode      GetInterpolationMode() const { return mMode; }
        NODISCARD INLINE void                   SetInterpolationMode(InterpolationMode mode) { mMode = mode; }

        NODISCARD float                         GetStartTime() const;
        NODISCARD float                         GetEndTime() const;
        NODISCARD INLINE UINT                   GetFirstFrame(UINT framesPerSecond) const { return (UINT)(GetStartTime() * framesPerSecond); }
        NODISCARD INLINE UINT                   GetLastFrame(UINT framesPerSecond) const { return (UINT)(GetEndTime() * framesPerSecond); }

        NODISCARD INLINE UINT                   GetKeyframesCount() const { return (UINT)mKeyframes.size(); }
    protected:
        DField(Serialize)
        D_CONTAINERS::DVector<Keyframe>         mKeyframes;

        DField(Serialize)
        InterpolationMode                       mMode;

        DField(Serialize)
        KeyframeDataType                        mKeyframeDataType;

    };

    // Contains multiple tracks/curves
    class DClass(Serialize) Sequence
    {
        GENERATED_BODY();
    public:
        Sequence() = default;
        virtual ~Sequence() = default;
        
        INLINE Track const*                     GetTrack(std::string const& name) const { return mTracksNameIndex.contains(name) ? mTracks.at(mTracksNameIndex.at(name)) : nullptr; }
        
        template<typename T>
        std::optional<T>                        Evaluate(std::string const& name, float time, bool extrapolateLastValue = false);

        UINT                                    AddTrack(std::string const& name, Track const& track);
        bool                                    RemoveTrack(std::string const& name);

        D_CONTAINERS::DUnorderedMap<std::string, UINT> const& GetNameIndexMapping() const { return mTracksNameIndex; }

        D_CONTAINERS::DVector<Track*> const&     GetTracks() const { return mTracks; }

        NODISCARD INLINE float                  GetDuration() const { return GetEndTime() - GetStartTime(); }
        NODISCARD float                         GetStartTime() const;
        NODISCARD float                         GetEndTime() const;

    protected:

        DField(Serialize)
        D_CONTAINERS::DUnorderedMap<std::string, UINT> mTracksNameIndex;

        DField(Serialize)
        D_CONTAINERS::DVector<Track>            mTracks;

        float                                   mStartTime = 0.f;
        float                                   mEndTime = 0.f;
    };

    template<typename T>
    std::optional<T> Sequence::Evaluate(std::string const& name, float time, bool extrapolateLastValue)
    {
        Track const* track = GetTrack(name);
        if (!track)
            return std::optional<T>();

        return track->Evaluate<T>(time, extrapolateLastValue);
    }

    template<typename T>
    std::optional<T> Track::Evaluate(float time, bool extrapolateLastValues) const
    {
        size_t count = mKeyframes.size();

        if (count == 0)
            return std::optional<T>();

        if (time <= mKeyframes[0].Time)
            return std::optional(mKeyframes[0].GetValue<T>());

        if (count == 1 || time >= mKeyframes[count - 1].Time)
        {
            if (extrapolateLastValues)
                return std::optional(mKeyframes[count - 1].GetValue<T>());
            else
                return std::optional<T>();
        }

        for (size_t offset = 0; offset < count; offset++)
        {
            const float tb = mKeyframes[offset].Time;
            const float tc = mKeyframes[offset + 1].Time;

            if (tb <= time && time < tc)
            {
                Keyframe const& b = mKeyframes[offset];
                Keyframe const& c = mKeyframes[offset + 1];
                Keyframe const& a = (offset > 0) ? mKeyframes[offset - 1] : b;
                Keyframe const& d = (offset < count - 2) ? mKeyframes[offset + 2] : c;
                const float dt = tc - tb;
                const float u = (time - tb) / dt;

                T y = Interpolate<T>(mMode, a, b, c, d, u, dt);

                return std::optional(y);
            }
        }

        D_ASSERT_M(true, "Keyframes are not properly ordered by their time.");
        return std::optional<T>();
    }

}
