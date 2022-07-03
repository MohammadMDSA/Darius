#pragma once

#include <Math/Transform.hpp>
#include "DeviceResources.hpp"

#define D_RENDERER Darius::Renderer
#define D_RENDERER_DEVICE Darius::Renderer::Device
#define D_DEVICE D_RENDERER_DEVICE

using namespace Darius::Math;
using namespace Darius::Renderer::DeviceResource;

namespace Darius::Renderer
{
	// For now, just a simple cube draw
	// Definitely bad input signature but we will get there...
	void Initialize();
	void Shutdown();

	void RenderMeshes();
	void Update(Transform* trans);


	namespace Device
	{
		void RegisterDeviceNotify(IDeviceNotify* notify);
		void Initialize(HWND window, int width, int height);
		void Shutdown();

		// Window functions
		void OnWindowMoved();
		void OnDisplayChanged();
		bool OnWindowsSizeChanged(int width, int height);
		void ShaderCompatibilityCheck(D3D_SHADER_MODEL shaderModel);
	}
}