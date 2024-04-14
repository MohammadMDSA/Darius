#include "Math/pch.hpp"
#include "ShadowCamera.hpp"

namespace Darius::Math::Camera
{
    void ShadowCamera::UpdateMatrix(
        Vector3 const& LightDirection, Vector3 const& ShadowCenter, Vector3 const& ShadowBounds,
        uint32_t BufferWidth, uint32_t BufferHeight, uint32_t BufferPrecision)
    {
        SetLookDirection(LightDirection, Vector3::Up);

        // Converts world units to texel units so we can quantize the camera position to whole texel units
        Vector3 RcpDimensions = Recip(ShadowBounds);
        Vector3 QuantizeScale = Vector3((float)BufferWidth, (float)BufferHeight, (float)((1 << BufferPrecision) - 1)) * RcpDimensions;

        //
        // Recenter the camera at the quantized position
        //

        // Transform to view space
        Vector3 center = ~GetRotation() * ShadowCenter;
        // Scale to texel units, truncate fractional part, and scale back to world units
        center = Floor(center * QuantizeScale) / QuantizeScale;
        // Transform back into world space
        center = GetRotation() * center;
        SetPosition(center);

        Vector3 scale = Vector3(2.f, 2.f, 1.f) * RcpDimensions;
        SetProjMatrix(Matrix4(DirectX::XMMatrixOrthographicRH(ShadowBounds.GetX(), ShadowBounds.GetY(), ShadowBounds.GetZ(), 0.f)));

        Update();

        // Transform from clip space to texture space
        m_ShadowMatrix = Matrix4(AffineTransform(Matrix3::MakeScale(0.5f, -0.5f, 1.0f), Vector3(0.5f, 0.5f, 0.0f))) * m_ViewProjMatrix;
    }
}
