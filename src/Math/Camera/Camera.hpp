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
#include "Math/Ray.hpp"
#include "Frustum.hpp"

#include <Utils/Common.hpp>

#include <rttr/rttr_enable.h>

#include "Camera.generated.hpp"

#define MIN_ORTHO_SIZE 0.01f


// Base camera class was registered manually, so needs the generated declarations too
static void rttr_auto_register_reflection_function_BaseCamera_();

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
        void SetTransform(AffineTransform const& xform);
        void SetTransform(OrthogonalTransform const& xform);

        INLINE Quaternion GetRotation() const { return m_CameraToWorld.GetRotation(); }
        INLINE Vector3 GetRightVec() const { return m_Basis.GetX(); }
        INLINE Vector3 GetUpVec() const { return m_Basis.GetY(); }
        INLINE Vector3 GetForwardVec() const { return -m_Basis.GetZ(); }
        INLINE Vector3 GetPosition() const { return m_CameraToWorld.GetTranslation(); }
        INLINE Ray GetCameraRay() const { return Ray(GetPosition(), GetForwardVec()); }
        INLINE bool IsMirrored() const { return m_ViewMatrix.Get3x3().Determinant() < 0.f; }

        // Accessors for reading the various matrices and frusta
        const Matrix4& GetViewMatrix() const { return m_ViewMatrix; }
        const Matrix4& GetProjMatrix() const { return m_ProjMatrix; }
        const Matrix4& GetViewProjMatrix() const { return m_ViewProjMatrix; }
        const Matrix4& GetReprojectionMatrix() const { return m_ReprojectMatrix; }
        const Frustum& GetViewSpaceFrustum() const { return m_FrustumVS; }
        const Frustum& GetWorldSpaceFrustum() const { return m_FrustumWS; }

    protected:

        BaseCamera() :
            m_CameraToWorld(kIdentity),
            m_Basis(kIdentity),
            mFrustumVSDirty(true),
            mFrustumWSDirty(true)
        {}

        void SetProjMatrix(const Matrix4& ProjMat)
        {
            m_ProjMatrix = ProjMat;
            mFrustumVSDirty = true;
        }

        OrthogonalTransform m_CameraToWorld;
        OrthogonalTransform m_PrevCameraToWorld;

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

    private:
        UINT mFrustumVSDirty : 1;
        UINT mFrustumWSDirty : 1;

        RTTR_REGISTRATION_FRIEND_PFX(BaseCamera);
        RTTR_ENABLE();

    };

    class DClass(Serialize) Camera : public BaseCamera
    {
        GENERATED_BODY();

    public:
        Camera();

        // Controls the view-to-projection matrix
        void            SetPerspectiveMatrix(float verticalFovRadians, float aspectHeightOverWidth, float nearZClip, float farZClip);
        void            SetFoV(float verticalFovInRadians) { mVerticalFoV = verticalFovInRadians; UpdateProjMatrix(); }
        INLINE void     SetAspectRatio(float heightOverWidth) { mAspectRatio = heightOverWidth; UpdateProjMatrix(); }
        void            SetZRange(float nearZ, float farZ) { mNearClip = nearZ; mFarClip = farZ; UpdateProjMatrix(); }
        void            ReverseZ(bool enable) { mReverseZ = enable; UpdateProjMatrix(); }
        INLINE void     SetOrthographic(bool isOrthoGraphic) { mOrthographic = isOrthoGraphic; UpdateProjMatrix(); }
        INLINE void     SetOrthographicSize(float size)
        {
            size = D_MATH::Max(size, MIN_ORTHO_SIZE);
            if (size == mOrthographicSize)
                return;

            mOrthographicSize = size;
            UpdateProjMatrix();
        }

        INLINE void     SetInfiniteZ(bool val) { mInfiniteZ = val; UpdateProjMatrix(); }

        float           GetFoV() const { return mVerticalFoV; }
        float           GetClearDepth() const { return mReverseZ ? 0.0f : 1.0f; }
        INLINE float    GetAspectRatio() const { return mAspectRatio; }
        INLINE float    GetNearClip() const { return mNearClip; }
        INLINE float    GetFarClip() const { return mFarClip; }
        INLINE float    GetOrthographicSize() const { return mOrthographicSize; }
        INLINE bool     IsReverseZ() const { return mReverseZ; }
        INLINE bool     IsInfiniteZ() const { return mInfiniteZ; }
        INLINE bool     IsOrthographic() const { return mOrthographic; }

        void            UpdateProjMatrix();

        // Does not support infinite Z
        void            CalculateSlicedFrustumes(D_CONTAINERS::DVector<float> const& ranges, D_CONTAINERS::DVector<Frustum>& slicedFrustums) const;

        static Matrix4 CalculatePerspectiveProjectionMatrix(float verticalFoV, float aspectRatio, float nearClip, float farClip, bool reverseZ, bool infiniteZ);
        static Matrix4 CalculateOrthographicProjectionMatrix(float orthographicSize, float aspectRatio, float nearClip, float farClip, float reverseZ);
    private:

        DField(Serialize)
        float mVerticalFoV;	// Field of view angle in radians

        DField(Serialize)
        float mAspectRatio;

        DField(Serialize)
        float mNearClip;

        DField(Serialize)
        float mFarClip;

        DField(Serialize)
        float mOrthographicSize;

        DField(Serialize)
        bool mReverseZ;		// Invert near and far clip distances so that Z=1 at the near plane

        DField(Serialize)
        bool mInfiniteZ;       // Move the far plane to infinity

        DField(Serialize)
        bool mOrthographic;

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