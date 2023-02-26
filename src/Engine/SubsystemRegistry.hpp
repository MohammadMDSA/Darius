#pragma once

#include <Core/Filesystem/Path.hpp>

#ifndef D_SUBSYSTEMS
#define D_SUBSYSTEMS Darius::Subsystems
#endif

namespace Darius::Subsystems
{
	void RegisterSubsystems();
	void InitialzieSubsystems(HWND window, int width, int height, D_FILE::Path const& projectPath);
	void ShutdownSubsystems();

}
