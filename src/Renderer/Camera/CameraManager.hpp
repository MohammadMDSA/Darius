#pragma once

#define D_CAMERA_MANAGER Darius::Renderer::CameraManager

#include <Math/Camera/Camera.hpp>

using namespace D_MATH_CAMERA;

namespace Darius::Renderer::CameraManager
{
	void Initialize();
	void Shutdown();

	// Active camera functions
	void SetActiveCamera(D_MATH_CAMERA::Camera* cam, int index = 0);
	D_MATH_CAMERA::Camera* GetActiveCamera(int index = 0);

	// Viewport dimansion functions
	void SetViewportDimansion(float w, float h);
	void GetViewportDimansion(float& width, float& height);
}