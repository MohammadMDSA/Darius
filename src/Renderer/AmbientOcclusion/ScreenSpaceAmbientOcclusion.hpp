#pragma once

#include "Renderer/CommandContext.hpp"
#include "Renderer/GraphicsUtils/Buffers/ColorBuffer.hpp"
#include "Renderer/GraphicsUtils/Buffers/DepthBuffer.hpp"

#include <Core/Serialization/Json.hpp>
#include <Math/VectorMath.hpp>
#include <Math/Camera/Camera.hpp>

#ifndef D_GRAPHICS_AO_SS
#define D_GRAPHICS_AO_SS Darius::Graphics::AmbientOcclusion::ScreenSpace
#endif

namespace Darius::Graphics::AmbientOcclusion::ScreenSpace
{

    struct SSAORenderBuffers
    {
        D_GRAPHICS_BUFFERS::ColorBuffer& SceneColorBuffer;
        D_GRAPHICS_BUFFERS::ColorBuffer& SSAOFullScreen;
        D_GRAPHICS_BUFFERS::DepthBuffer& Depth;
        D_GRAPHICS_BUFFERS::ColorBuffer& LinearDepth;
        D_GRAPHICS_BUFFERS::ColorBuffer& DepthDownsize1;
        D_GRAPHICS_BUFFERS::ColorBuffer& DepthDownsize2;
        D_GRAPHICS_BUFFERS::ColorBuffer& DepthDownsize3;
        D_GRAPHICS_BUFFERS::ColorBuffer& DepthDownsize4;
        D_GRAPHICS_BUFFERS::ColorBuffer& DepthTiled1;
        D_GRAPHICS_BUFFERS::ColorBuffer& DepthTiled2;
        D_GRAPHICS_BUFFERS::ColorBuffer& DepthTiled3;
        D_GRAPHICS_BUFFERS::ColorBuffer& DepthTiled4;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOMerged1;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOMerged2;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOMerged3;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOMerged4;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOSmooth1;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOSmooth2;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOSmooth3;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOHighQuality1;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOHighQuality2;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOHighQuality3;
        D_GRAPHICS_BUFFERS::ColorBuffer& AOHighQuality4;
    };

    void                        Initialize(D_SERIALIZATION::Json const& settings);
    void                        Shutdown();

    void                        Render(D_GRAPHICS::GraphicsContext& context, SSAORenderBuffers& buffers, D_MATH::Matrix4 const& projMat, float nearClip, float farClip);
    void                        Render(D_GRAPHICS::GraphicsContext& context, SSAORenderBuffers& buffers, D_MATH_CAMERA::Camera const& camera);

    void                        LinearizeZ(D_GRAPHICS::ComputeContext& context, D_GRAPHICS_BUFFERS::DepthBuffer& depth, D_GRAPHICS_BUFFERS::ColorBuffer& linearDepth, D_MATH_CAMERA::Camera const& camera);

#ifdef _D_EDITOR
    bool                        OptionsDrawer(_IN_OUT_ D_SERIALIZATION::Json& options);
#endif
}
