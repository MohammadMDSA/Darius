#pragma once

#include <Utils/Common.hpp>

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif // !D_GRAPHICS


namespace Darius::Graphics
{
	class D3D12Viewport : public NonCopyable
	{
	public:
		D3D12Viewport(
			HWND wind,
			DXGI_FORMAT backbufferFormat,
			DXGI_FORMAT depthBufferFormat,
			UINT backbufferCount = 2
			);
		~D3D12Viewport();

		void CreateViewport();

		void SetWindow(HWND window, int width, int heght);

	};
}
