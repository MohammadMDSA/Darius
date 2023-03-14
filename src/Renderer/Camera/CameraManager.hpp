#pragma once

#define D_CAMERA_MANAGER Darius::Renderer::CameraManager

#include "Renderer/Components/CameraComponent.hpp"

#include <Scene/EntityComponentSystem/CompRef.hpp>
#include <Math/Camera/Camera.hpp>

using namespace D_MATH_CAMERA;

namespace Darius::Renderer::CameraManager
{
	void Initialize();
	void Shutdown();
	void Update();

	// Active camera functions
	void SetActiveCamera(D_ECS::CompRef<CameraComponent> cam, int index = 0);
	D_ECS::CompRef<CameraComponent> GetActiveCamera(int index = 0);

	void RegisterCamera(D_ECS::CompRef<CameraComponent> cam);

	// Viewport dimansion functions
	bool SetViewportDimansion(float w, float h);
	void GetViewportDimansion(float& width, float& height);
	float GetViewportAspectRatio();
}