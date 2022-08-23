#pragma once
#include "pch.hpp"
#include "DeviceResources.hpp"

#define D_RENDERER_DEVICE Darius::Renderer::Device

using namespace Darius::Renderer::DeviceResource;

namespace Darius::Renderer::Device
{
	void RegisterDeviceNotify(IDeviceNotify* notify);
	void Initialize(HWND window, int width, int height);
	void Shutdown();

	FrameResource*	GetCurrentFrameResource();
	ID3D12Device*	GetDevice();
	FrameResource*	GetFrameResourceWithIndex(int i);
	DXGI_FORMAT		GetBackBufferFormat();
	DXGI_FORMAT		GetDepthBufferFormat();

	// Window functions
	void OnWindowMoved();
	void OnDisplayChanged();
	bool OnWindowsSizeChanged(int width, int height);
	void ShaderCompatibilityCheck(D3D_SHADER_MODEL shaderModel);
	UINT GetCurrentResourceIndex();
}