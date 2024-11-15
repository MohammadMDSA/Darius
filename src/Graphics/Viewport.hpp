#pragma once

#include <Core/Containers/Vector.hpp>
#include <Math/Rectangle.hpp>

#include <Utils/Common.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif // !D_GRAPHICS

namespace Darius::Graphics
{
    static constexpr uint32_t MaxViewports = 16u;

    struct Viewport
    {
        float MinX, MaxX;
        float MinY, MaxY;
        float MinZ, MaxZ;

        Viewport() :
            MinX(0.f),
            MaxX(0.f),
            MinY(0.f),
            MaxY(0.f),
            MinZ(0.f),
            MaxZ(1.f)
        { }

        Viewport(float width, float height) :
            MinX(0.f),
            MaxX(width),
            MinY(0.f),
            MaxY(height),
            MinZ(0.f),
            MaxZ(1.f)
        { }

        Viewport(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) :
            MinX(minX),
            MaxX(maxX),
            MinY(minY),
            MaxY(maxY),
            MinZ(minZ),
            MaxZ(maxZ)
        { }

        bool operator ==(Viewport const& b) const
        {
            return MinX == b.MinX
                && MinY == b.MinY
                && MinZ == b.MinZ
                && MaxX == b.MaxX
                && MaxY == b.MaxY
                && MaxZ == b.MaxZ;
        }
        bool operator !=(const Viewport& b) const { return !(*this == b); }

        NODISCARD float Width() const { return MaxX - MinX; }
        NODISCARD float Height() const { return MaxY - MinY; }

        INLINE D_MATH::Rect Rect() const
        {
            return D_MATH::Rect(
                long(floorf(MinX)),
                long(ceilf(MaxX)),
                long(floorf(MinY)),
                long(ceilf(MaxY))
            );
        }
    };

    struct ViewportState
    {
        D_CONTAINERS::DVector<Viewport>             Viewports;
        D_CONTAINERS::DVector<D_MATH::Rectangle>    ScissorRects;

        ViewportState& AddViewport(Viewport const& viewport)
        {
            Viewports.push_back(viewport);
            return *this;
        }

        ViewportState& AddScissorRect(D_MATH::Rectangle const& scissorRect)
        {
            ScissorRects.push_back(scissorRect);
            return *this;
        }

        ViewportState& AddViewportAndScissorRect(Viewport const& viewport, D_MATH::Rectangle const& rect)
        {
            return AddViewport(viewport).AddScissorRect(rect);
        }
    };
}
