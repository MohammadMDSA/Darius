#pragma once
#include "pch.hpp"
#include "DeviceResources.hpp"

#ifndef D_GRAPHICS_DEVICE
#define D_GRAPHICS_DEVICE Darius::Graphics::Device
#endif // !D_GRAPHICS_DEVICE

namespace Darius::Graphics::Device
{
	void										RegisterDeviceNotify(void*);

	ID3D12Device*								GetDevice();
	ID3D12Device5*								GetDevice5();
	D3DX12Residency::ResidencyManager&			GetResidencyManager();
	UINT										GetCurrentFrameResourceIndex();

	UINT										GetBackBufferCount();
	D_GRAPHICS_BUFFERS::ColorBuffer&			GetRTBuffer();
	D_GRAPHICS_BUFFERS::DepthBuffer&			GetDepthStencilBuffer();
	RECT										GetOutputSize();

	// Support Check
	bool										SupportsTypedUAVLoadSupport_R11G11B10_FLOAT();
	bool										SupportsTypedUAVLoadSupport_R16G16B16A16_FLOAT();
	bool										SupportsRaytracing();


	// Window functions
	void										OnWindowMoved();
	void										OnDisplayChanged();
	bool										OnWindowsSizeChanged(int width, int height);
	void										ShaderCompatibilityCheck(D3D_SHADER_MODEL shaderModel);
}