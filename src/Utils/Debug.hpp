#pragma once
#include <string>

#define D_DEBUG Darius::Utils::Debug

namespace Darius::Utils::Debug
{
#ifdef _DEBUG

	std::string GetLatestWinPixGpuCapturerPath();

	void AttachWinPixGpuCapturer();

#endif
}