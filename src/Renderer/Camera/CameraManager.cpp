#include "../pch.hpp"

#include "CameraManager.hpp"

#include <Utils/Assert.hpp>


namespace Darius::Renderer::CameraManager
{

	float _initialized = false;

	float Width = 1, Height = 1;
	std::array<D_MATH_CAMERA::Camera*, 10> activeCameras;

	void Initialize()
	{
		D_ASSERT(!_initialized);

		for (size_t i = 0; i < activeCameras.size(); i++)
		{
			activeCameras[i] = nullptr;
		}

		_initialized = true;
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
	}

	void SetActiveCamera(D_MATH_CAMERA::Camera* cam, int index)
	{
		activeCameras[index] = cam;
		if (cam)
			cam->SetViewSize(Width, Height);
	}

	D_MATH_CAMERA::Camera* GetActiveCamera(int index)
	{
		return activeCameras[index];
	}

	bool SetViewportDimansion(float w, float h)
	{
		w = XMMax(w, 1.f);
		h = XMMax(h, 1.f);
		if (w == Width && h == Height)
			return false;
		Width = w;
		Height = h;

		for (auto cam : activeCameras)
			if (cam)
				cam->SetViewSize(w, h);

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