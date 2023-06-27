#include "../pch.hpp"

#include "CameraManager.hpp"

#include <Utils/Assert.hpp>

using namespace D_GRAPHICS;

namespace Darius::Renderer::CameraManager
{

	float _initialized = false;

	float Width = 1, Height = 1;
	std::array<D_ECS::CompRef<CameraComponent>, 10> activeCameras;

	void Initialize()
	{
		D_ASSERT(!_initialized);

		_initialized = true;
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

	void Update()
	{
		for (auto cam : activeCameras)
			if (cam.IsValid() && cam->IsActive())
				cam->Update(-1.f);
	}

	void SetActiveCamera(D_ECS::CompRef<CameraComponent> cam, int index)
	{
		activeCameras[index] = cam;
		if (cam.IsValid())
			cam->SetAspectRatio(Height / Width);
	}

	D_ECS::CompRef<CameraComponent> GetActiveCamera(int index)
	{
		return activeCameras[index];
	}

	bool SetViewportDimansion(float w, float h)
	{
		w = std::max(w, 1.f);
		h = std::max(h, 1.f);
		if (w == Width && h == Height)
			return false;
		Width = w;
		Height = h;

		for (auto cam : activeCameras)
			if (cam.IsValid())
				cam->SetAspectRatio(h / w);

		return true;
	}

	void GetViewportDimansion(float& width, float& height)
	{
		width = Width;
		height = Height;
	}

	float GetViewportAspectRatio()
	{
		return Width / Height;
	}

}