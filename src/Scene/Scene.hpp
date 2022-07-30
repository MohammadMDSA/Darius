#pragma once

#include "GameObject.hpp"

#ifndef D_SCENE
#define D_SCENE Darius::Scene
#endif // !D_SCENE

namespace Darius::Scene
{
	void Initialize();
	void Shutdown();

	bool Create(std::string& name);
	void Unload();
	bool Load(std::wstring& path);
	bool Save(std::string& name, std::wstring& path);

}