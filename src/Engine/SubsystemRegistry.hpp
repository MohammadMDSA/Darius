#pragma once

#include <Core/Containers/Map.hpp>
#include <Core/Filesystem/Path.hpp>
#include <Core/Serialization/Json.hpp>

#include <functional>

#ifndef D_SUBSYSTEMS
#define D_SUBSYSTEMS Darius::Subsystems
#endif

namespace Darius::Subsystems
{
	void RegisterSubsystems();
	void InitialzieSubsystems(HWND window, int width, int height, D_FILE::Path const& projectPath);
	void ShutdownSubsystems();
#ifdef _D_EDITOR
	void GetSubsystemsOptionsDrawer(D_CONTAINERS::DUnorderedMap<std::string, std::function<bool(D_SERIALIZATION::Json&)>>& systemOption);
#endif
}
