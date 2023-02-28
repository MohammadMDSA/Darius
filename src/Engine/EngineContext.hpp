#pragma once

#include <Core/Filesystem/Path.hpp>

#include <Utils/Common.hpp>

#ifndef D_ENGINE_CONTEXT
#define D_ENGINE_CONTEXT Darius::Engine::Context
#endif // D_ENGINE_CONTEXT

namespace Darius::Engine::Context
{
	void Initialize(D_FILE::Path const& projectPath, HWND window, int width, int height);
	void Shutdown();

	D_FILE::Path GetProjectPath();
	D_FILE::Path GetAssetsPath();
	D_FILE::Path GetEngineConfigPath();
	D_FILE::Path GetEngineSettingsPath();
}
