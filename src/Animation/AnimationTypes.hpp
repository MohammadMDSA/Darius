#pragma once

#include <Core/Containers/Map.hpp>

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

namespace Darius::Animation
{

    // An animation curve describes how a value (or values) change over time.
    // Key frames punctuate the curve, and times inbetween key frames are interpolated
    // using the selected method.  Note that a curve does not have to be defined for
    // the entire animation.  Some property might only be animated for a portion of time.
    struct AnimationCurve
    {
        enum { kTranslation, kRotation, kScale, kWeights }; // targetPath
        enum { kLinear, kStep, kCatmullRomSpline, kCubicSpline }; // interpolation
        enum { kSNorm8, kUNorm8, kSNorm16, kUNorm16, kFloat }; // format

        uint32_t    TargetNode : 28;           // Which node is being animated
        uint32_t    TargetPath : 2;            // What aspect of the transform is animated
        uint32_t    Interpolation : 2;         // The method of interpolation
        uint32_t    KeyFrameOffset : 26;       // Byte offset to first key frame
        uint32_t    KeyFrameFormat : 3;        // Data format for the key frames
        uint32_t    KeyFrameStride : 3;        // Number of 4-byte words for one key frame
        float       StartTime;                    // Time stamp of the first key frame
        D_CONTAINERS::DMap<float, int> KeyframeTimeMap; // Mapping from keyframe time to index
    };

    //
    // An animation is composed of multiple animation curves.
    //
    struct AnimationLayer
    {
        float       Duration = -1;
    };

    typedef uint8_t Keyframe;

}
