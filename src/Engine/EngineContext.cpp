#include "pch.hpp"
#include "EngineContext.hpp"

#include "SubsystemRegistry.hpp"

#include <Utils/Assert.hpp>

namespace Darius::Engine::Context
{
	bool							_initialized = false;
	D_FILE::Path					ProjectPath;

	void Initialize(D_FILE::Path const& projectPath, HWND window, int width, int height)
	{
		D_ASSERT(!_initialized);
		_initialized = true;

		ProjectPath = projectPath;

		D_ASSERT_M(D_H_ENSURE_DIR(projectPath), "Project directory is not a valid directory");

		if (!D_H_ENSURE_PATH(GetAssetsPath()))
		{
			D_ASSERT_M(std::filesystem::create_directory(GetAssetsPath()), "Could not detect/create assets directory for project");
		}

		if (!D_H_ENSURE_PATH(GetEngineConfigPath()))
		{
			D_ASSERT_M(std::filesystem::create_directory(GetEngineConfigPath()), "Could not detect/create engine config directory for project");
		}

		// Registering all subsystems
		D_SUBSYSTEMS::RegisterSubsystems();

		// Initializing engine subsystems
		D_SUBSYSTEMS::InitialzieSubsystems(window, width, height, projectPath);

	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		// Shuting down engine subsystems
		D_SUBSYSTEMS::ShutdownSubsystems();
	}

	D_FILE::Path GetProjectPath()
	{
		return ProjectPath;
	}

	D_FILE::Path GetAssetsPath()
	{
		return D_FILE::Path(ProjectPath).append("Assets/");
	}

	D_FILE::Path GetEngineConfigPath()
	{
		return D_FILE::Path(ProjectPath).append("Config/Engine/");
	}

	D_FILE::Path GetEngineSettingsPath()
	{
		return GetEngineConfigPath().append("Settings.json");
	}
}
