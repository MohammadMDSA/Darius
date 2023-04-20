//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once

#include "Math/VectorMath.hpp"
#include "Frustum.hpp"

#include <rttr/rttr_enable.h>

#include "Camera.generated.hpp"

namespace Darius::Math::Camera
{
    class DClass() BaseCamera
    {
    public:

        // Call this function once per frame and after you've changed any state.  This
        // regenerates all matrices.  Calling it more or less than once per frame will break
        // temporal effects and cause unpredictable results.
        void Update();

        // Public functions for controlling where the camera is and its orientation
        void SetEyeAtUp(Vector3 eye, Vector3 at, Vector3 up);
        void SetLookDirection(Vector3 forward, Vector3 up);
        void SetRotation(Quaternion basisRotation);
        void SetPosition(Vector3 worldPos);
        void SetTransform(const AffineTransform& xform);
        void SetTransform(const OrthogonalTransform& xform);

        Quaternion GetRotation() const { return m_CameraToWorld.GetRotation(); }
        Vector3 GetRightVec() const { return m_Basis.GetX(); }
        Vector3 GetUpVec() const { return m_Basis.GetY(); }
        Vector3 GetForwardVec() const { return -m_Basis.GetZ(); }
        Vector3 GetPosition() const { return m_CameraToWorld.GetTranslation(); }

        // Accessors for reading the various matrices and frusta
        const Matrix4& GetViewMatrix() const { return m_ViewMatrix; }
        const Matrix4& GetProjMatrix() const { return m_ProjMatrix; }
        const Matrix4& GetViewProjMatrix() const { return m_ViewProjMatrix; }
        const Matrix4& GetReprojectionMatrix() const { return m_ReprojectMatrix; }
        const Frustum& GetViewSpaceFrustum() const { return m_FrustumVS; }
        const Frustum& GetWorldSpaceFrustum() const { return m_FrustumWS; }

    protected:

        BaseCamera() : m_CameraToWorld(kIdentity), m_Basis(kIdentity) {}

        void SetProjMatrix(const Matrix4& ProjMat) { m_ProjMatrix = ProjMat; }

        OrthogonalTransform m_CameraToWorld;

        // Redundant data cached for faster lookups.
        Matrix3 m_Basis;

        // Transforms homogeneous coordinates from world space to view space.  In this case, view space is defined as +X is
        // to the right, +Y is up, and -Z is forward.  This has to match what the projection matrix expects, but you might
        // also need to know what the convention is if you work in view space in a shader.
        Matrix4 m_ViewMatrix;		// i.e. "World-to-View" matrix

        // The projection matrix transforms view space to clip space.  Once division by W has occurred, the final coordinates
        // can be transformed by the viewport matrix to screen space.  The projection matrix is determined by the screen aspect 
        // and camera field of view.  A projection matrix can also be orthographic.  In that case, field of view would be defined
        // in linear units, not angles.
        Matrix4 m_ProjMatrix;		// i.e. "View-to-Projection" matrix

        // A concatenation of the view and projection matrices.
        Matrix4 m_ViewProjMatrix;	// i.e.  "World-To-Projection" matrix.

        // The view-projection matrix from the previous frame
        Matrix4 m_PreviousViewProjMatrix;

        // Projects a clip-space coordinate to the previous frame (useful for temporal effects).
        Matrix4 m_ReprojectMatrix;

        Frustum m_FrustumVS;		// View-space view frustum
        Frustum m_FrustumWS;		// World-space view frustum

        RTTR_REGISTRATION_FRIEND;
        RTTR_ENABLE();

    public:
        Darius_Math_Camera_BaseCamera_GENERATED;
    };

    class DClass(Serialize) Camera : public BaseCamera
    {
    public:
        Camera();

        // Controls the view-to-projection matrix
        void            SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip);
        void            SetFoV(float verticalFovInRadians) { mVerticalFoV = verticalFovInRadians; UpdateProjMatrix(); }
        INLINE void     SetAspectRatio(float heightOverWidth) { mAspectRatio = heightOverWidth; UpdateProjMatrix(); }
        void            SetZRange(float nearZ, float farZ) { mNearClip = nearZ; mFarClip = farZ; UpdateProjMatrix(); }
        void            ReverseZ(bool enable) { mReverseZ = enable; UpdateProjMatrix(); }
        INLINE void     SetOrthographic(bool isOrthoGraphic) { mOrthographic = isOrthoGraphic; UpdateProjMatrix(); }
        INLINE void     SetOrthographicSize(float size) { mOrthographicSize = size; UpdateProjMatrix(); }
        INLINE void     SetInfiniteZ(bool val) { mInfiniteZ = val; UpdateProjMatrix(); }

        float           GetFoV() const { return mVerticalFoV; }
        float           GetClearDepth() const { return mReverseZ ? 0.0f : 1.0f; }

        void            UpdateProjMatrix();
    private:

        DField(Serialize)
        float mVerticalFoV;	// Field of view angle in radians

        DField(Get[const, &, inline], Serialize)
        float mAspectRatio;

        DField(Get[const, &, inline], Serialize)
        float mNearClip;

        DField(Get[const, &, inline], Serialize)
        float mFarClip;

        DField(Get[const, &, inline], Serialize)
        float mOrthographicSize;

        DField(Get[const, &, inline], Serialize)
        bool mReverseZ;		// Invert near and far clip distances so that Z=1 at the near plane

        DField(Get[const, &, inline], Serialize)
        bool mInfiniteZ;       // Move the far plane to infinity

        DField(Get[const, &, inline], Serialize)
        bool mOrthographic;

    public:
        Darius_Math_Camera_Camera_GENERATED;
    };

    inline void BaseCamera::SetEyeAtUp(Vector3 eye, Vector3 at, Vector3 up)
    {
        SetLookDirection(at - eye, up);
        SetPosition(eye);
    }

    inline void BaseCamera::SetPosition(Vector3 worldPos)
    {
        m_CameraToWorld.SetTranslation(worldPos);
    }

    inline void BaseCamera::SetTransform(const AffineTransform& xform)
    {
        // By using these functions, we rederive an orthogonal transform.
        SetLookDirection(-xform.GetZ(), xform.GetY());
        SetPosition(xform.GetTranslation());
    }

    inline void BaseCamera::SetTransform(const OrthogonalTransform& xform)
    {
        SetRotation(xform.GetRotation());
        SetPosition(xform.GetTranslation());
    }

    inline void BaseCamera::SetRotation(Quaternion basisRotation)
    {
        m_CameraToWorld.SetRotation(Normalize(basisRotation));
        m_Basis = Matrix3(m_CameraToWorld.GetRotation());
    }

    inline Camera::Camera() : mReverseZ(true), mInfiniteZ(false), mOrthographic(false), mOrthographicSize(10)
    {
        SetPerspectiveMatrix(DirectX::XM_PIDIV4, 9.0f / 16.0f, 1.0f, 1000.0f);
    }

    inline void Camera::SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip)
    {
        mVerticalFoV = verticalFovRadians;
        mAspectRatio = aspectHeightOverWidth;
        mNearClip = nearZClip;
        mFarClip = farZClip;
        mOrthographic = false;

        UpdateProjMatrix();

        m_PreviousViewProjMatrix = m_ViewProjMatrix;
    }

} // namespace Math

File_Camera_GENERATED;