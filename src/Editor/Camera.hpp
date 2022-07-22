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

#include <Math/VectorMath.hpp>
#include <Math/Bounds/BoundingSphere.hpp>
#include <Math/Camera/Camera.hpp>

#ifndef D_EDITOR
#define D_EDITOR Darius::Editor
#endif // !D_EDITOR

namespace Darius::Editor
{
    class CameraController
    {
    public:
        // Assumes worldUp is not the X basis vector
        CameraController(D_MATH_CAMERA::Camera& camera) : m_TargetCamera(camera) {}
        virtual ~CameraController() {}
        virtual void Update(float dt) = 0;

        // Helper function
        static void ApplyMomentum(float& oldValue, float& newValue, float deltaTime);

    protected:
        D_MATH_CAMERA::Camera& m_TargetCamera;

    private:
        CameraController& operator=(const CameraController&) { return *this; }
    };

    class FlyingFPSCamera : public CameraController
    {
    public:
        FlyingFPSCamera(D_MATH_CAMERA::Camera& camera, D_MATH::Vector3 worldUp);

        virtual void Update(float dt) override;

        void SlowMovement(bool enable) { m_FineMovement = enable; }
        void SlowRotation(bool enable) { m_FineRotation = enable; }

        void EnableMomentum(bool enable) { m_Momentum = enable; }

        void SetHeadingPitchAndPosition(float heading, float pitch, const D_MATH::Vector3& position);

    private:
        FlyingFPSCamera& operator=(const FlyingFPSCamera&) { return *this; }
        D_MATH::Vector3 m_WorldUp;
        D_MATH::Vector3 m_WorldNorth;
        D_MATH::Vector3 m_WorldEast;
        float m_HorizontalLookSensitivity;
        float m_VerticalLookSensitivity;
        float m_MoveSpeed;
        float m_StrafeSpeed;
        float m_MouseSensitivityX;
        float m_MouseSensitivityY;

        float m_CurrentHeading;
        float m_CurrentPitch;

        bool m_FineMovement;
        bool m_FineRotation;
        bool m_Momentum;

        float m_LastYaw;
        float m_LastPitch;
        float m_LastForward;
        float m_LastStrafe;
        float m_LastAscent;
    };

    class OrbitCamera : public CameraController
    {
    public:
        OrbitCamera(D_MATH_CAMERA::Camera& camera,
            D_MATH_BOUNDS::BoundingSphere focus,
            D_MATH::Vector3 upVec = D_MATH::Vector3(D_MATH::kYUnitVector));

        virtual void Update(float dt) override;

        void EnableMomentum(bool enable) { m_Momentum = enable; }

    private:
        OrbitCamera& operator=(const OrbitCamera&) { return *this; }

        D_MATH_BOUNDS::BoundingSphere m_ModelBounds;
        D_MATH::Vector3 m_WorldUp;

        float m_JoystickSensitivityX;
        float m_JoystickSensitivityY;

        float m_MouseSensitivityX;
        float m_MouseSensitivityY;

        float m_CurrentHeading;
        float m_CurrentPitch;
        float m_CurrentCloseness;

        bool m_Momentum;

        float m_LastYaw;
        float m_LastPitch;
        float m_LastForward;
    };
}