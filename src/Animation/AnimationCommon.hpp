#pragma once

#include <Core/Containers/Map.hpp>
#include <Core/Serialization/Json.hpp>
#include <Math/VectorMath.hpp>

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
        DField(Serialize)
        float               Time = 0.f;

        DField(Serialize)
        D_MATH::Vector4     Value = D_MATH::Vector4::Zero;

        DField(Serialize)
        D_MATH::Vector4     InTangent = D_MATH::Vector4::Zero;

        DField(Serialize)
        D_MATH::Vector4     OutTangent = D_MATH::Vector4::Zero;
    };

    enum class DEnum(Serialize) InterpolationMode
    {
        Step,
        Linear,
        Slerp,
        CatmullRomSpline,
        HermiteSpline
    };
    
    D_MATH::Vector4 Interpolate(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt);


    // Represents a curve on a scalar or vector value
    class DClass(Serialize) Track
    {
        GENERATED_BODY();
    public:
        Track() = default;
        virtual ~Track() = default;

        std::optional<D_MATH::Vector4>          Evaluate(float time, bool extrapolateLastValues = false) const;
        NODISCARD D_CONTAINERS::DVector<Keyframe>& GetKeyframes() { return mKeyframes; }
        Keyframe*                               AddKeyframe(Keyframe const& keyframe, int index = -1);

        // The pointer might invalidate upon change of the number of keyframes, so do not keep a reference
        NODISCARD Keyframe const*               FindKeyframeByTime(float time) const;
        // The pointer might invalidate upon change of the number of keyframes, so do not keep a reference
        NODISCARD Keyframe*                     FindKeyframeByTime(float time);
        // The pointer might invalidate upon change of the number of keyframes, so do not keep a reference
        NODISCARD Keyframe*                     FindOrCreateKeyframeByTime(float time);


        NODISCARD INLINE InterpolationMode      GetInterpolationMode() const { return mMode; }
        INLINE void                             SetInterpolationMode(InterpolationMode mode) { mMode = mode; }

        NODISCARD float                         GetStartTime() const;
        NODISCARD float                         GetEndTime() const;
    protected:
        DField(Serialize)
        D_CONTAINERS::DVector<Keyframe>         mKeyframes;

        DField(Serialize)
        InterpolationMode                       mMode = InterpolationMode::Linear;

    };

    // Contains multiple tracks/curves
    class DClass(Serialize) Sequence
    {
        GENERATED_BODY();
    public:
        Sequence() = default;
        virtual ~Sequence() = default;
        
        INLINE Track const*                     GetTrack(std::string const& name) const { return mTracksNameIndex.contains(name) ? &mTracks.at(mTracksNameIndex.at(name)) : nullptr; }
        
        std::optional<D_MATH::Vector4>          Evaluate(std::string const& name, float time, bool extrapolateLastValue = false);

        UINT                                    AddTrack(std::string const& name, Track const& track);

        NODISCARD INLINE float                  GetDuration() const { return mDuration; }

        D_CONTAINERS::DVector<Track> const&     GetTracks() const { return mTracks; }

    protected:

        DField(Serialize)
        D_CONTAINERS::DUnorderedMap<std::string, UINT> mTracksNameIndex;

        DField(Serialize)
        D_CONTAINERS::DVector<Track> mTracks;

        DField(Serialize)
        float                                   mDuration = 0.f;
    };

}
