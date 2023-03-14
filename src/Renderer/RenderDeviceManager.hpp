#pragma once
#include "pch.hpp"
#include "DeviceResources.hpp"

#define D_RENDERER_DEVICE Darius::Renderer::Device

namespace Darius::Renderer::Device
{
	void										RegisterDeviceNotify(D_DEVICE_RESOURCE::IDeviceNotify* notify);

	D_RENDERER_FRAME_RESOURCE::FrameResource*	GetCurrentFrameResource();
	ID3D12Device*								GetDevice();
	D_RENDERER_FRAME_RESOURCE::FrameResource*	GetFrameResourceWithIndex(int i);
	DXGI_FORMAT									GetBackBufferFormat();
	DXGI_FORMAT									GetDepthBufferFormat();

	// Window functions
	void										OnWindowMoved();
	void										OnDisplayChanged();
	bool										OnWindowsSizeChanged(int width, int height);
	void										ShaderCompatibilityCheck(D3D_SHADER_MODEL shaderModel);
	UINT										GetCurrentResourceIndex();
}