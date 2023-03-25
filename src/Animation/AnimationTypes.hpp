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

        uint32_t    TargetNode : 28 = 0;           // Which node is being animated
        uint32_t    TargetPath : 2 = 0;            // What aspect of the transform is animated
        uint32_t    Interpolation : 2 = 0;         // The method of interpolation
        uint32_t    KeyFrameOffset : 26 = 0;       // Byte offset to first key frame
        uint32_t    KeyFrameFormat : 3 = 0;        // Data format for the key frames
        uint32_t    KeyFrameStride : 3 = 0;        // Number of 4-byte words for one key frame
        float       StartTime = 0;                 // Time stamp of the first key frame
        int         ChannelIndex = 0;              // Component Index 
        D_CONTAINERS::DMap<float, UINT> KeyframeTimeMap = {}; // Mapping from keyframe time to index
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
