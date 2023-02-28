#pragma once

#include <Core/Filesystem/Path.hpp>
#include <Scene/GameObject.hpp>

#include <Utils/Detailed.hpp>

#ifndef D_EDITOR_CONTEXT
#define D_EDITOR_CONTEXT Darius::Editor::Context
#endif // !D_EDITOR_CONTEXT

using namespace D_FILE;
using namespace D_SCENE;

namespace Darius::Editor::Context
{
	void Initialize();
	void Shutdown();
	void Update(float elapsedTime);

	GameObject*						GetSelectedGameObject();
	void							SetSelectedGameObject(GameObject* go);
	Detailed*						GetSelectedDetailed();
	void							SetSelectedDetailed(Detailed* d);

	// Paths
	Path							GetProjectPath();
	Path							GetEditorConfigPath();
}
