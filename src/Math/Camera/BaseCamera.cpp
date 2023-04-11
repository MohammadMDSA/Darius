#include "../pch.hpp"
#include "Camera.hpp"

#include <cmath>
#include <rttr/registration.h>

using namespace DirectX;


namespace Darius::Math::Camera
{
    void BaseCamera::SetLookDirection(Vector3 forward, Vector3 up)
    {
        // Given, but ensure normalization
        Scalar forwardLenSq = LengthSquare(forward);
        forward = Select(forward * RecipSqrt(forwardLenSq), -Vector3(kZUnitVector), forwardLenSq < Scalar(0.000001f));

        // Deduce a valid, orthogonal right vector
        Vector3 right = Cross(forward, up);
        Scalar rightLenSq = LengthSquare(right);
        right = Select(right * RecipSqrt(rightLenSq), Quaternion(Vector3(kYUnitVector), -XM_PIDIV2) * forward, rightLenSq < Scalar(0.000001f));

        // Compute actual up vector
        up = Cross(right, forward);

        // Finish constructing basis
        m_Basis = Matrix3(right, up, -forward);
        m_CameraToWorld.SetRotation(Quaternion(m_Basis));
    }

    void BaseCamera::Update()
    {
        m_PreviousViewProjMatrix = m_ViewProjMatrix;

        m_ViewMatrix = Matrix4(~m_CameraToWorld);
        m_ViewProjMatrix = m_ProjMatrix * m_ViewMatrix;
        m_ReprojectMatrix = m_PreviousViewProjMatrix * Invert(GetViewProjMatrix());

        m_FrustumVS = Frustum(m_ProjMatrix);
        m_FrustumWS = m_CameraToWorld * m_FrustumVS;
    }
}

RTTR_REGISTRATION
{
    rttr::registration::class_<D_MATH_CAMERA::BaseCamera>("Darius::Math::Camera::BaseCamera")
        .property("Position", &D_MATH_CAMERA::BaseCamera::GetPosition, &D_MATH_CAMERA::BaseCamera::SetPosition)
        .property("Rotation",  &D_MATH_CAMERA::BaseCamera::GetRotation, &D_MATH_CAMERA::BaseCamera::SetRotation);
}
