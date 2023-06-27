#pragma once

#include "Graphics/CommandContext.hpp"
#include "Graphics/GraphicsUtils/Buffers/ColorBuffer.hpp"

#include <Core/Serialization/Json.hpp>
#include <Math/VectorMath.hpp>
#include <Math/Camera/Camera.hpp>

#ifndef D_GRAPHICS_PP_MOTION
#define D_GRAPHICS_PP_MOTION Darius::Graphics::PostProcessing::MotionBlur
#endif

namespace Darius::Graphics::PostProcessing::MotionBlur
{
    struct MotionBlurBuffers
    {
        D_GRAPHICS_BUFFERS::ColorBuffer&            SceneColorBuffer;
        D_GRAPHICS_BUFFERS::ColorBuffer&            LinearDepth;
        D_GRAPHICS_BUFFERS::ColorBuffer&            VelocityBuffer;
        D_GRAPHICS_BUFFERS::DepthBuffer&            SceneDepthBuffer;
    };

    void                Initialize(D_SERIALIZATION::Json const& settings);
    void                Shutdown();

    void                GenerateCameraVelocityBuffer(D_GRAPHICS::CommandContext& context, MotionBlurBuffers& buffers, D_MATH_CAMERA::Camera const& camera, bool useLinearZ = true);
    void                GenerateCameraVelocityBuffer(D_GRAPHICS::CommandContext& context, MotionBlurBuffers& buffers, D_MATH::Matrix4 const& reprojectionMatrix, float nearClip, float farClip, bool useLinearZ = true);
}
