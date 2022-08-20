#pragma once

#include <Core/Path.hpp>
#include <Scene/GameObject.hpp>

#ifndef D_EDITOR_CONTEXT
#define D_EDITOR_CONTEXT Darius::Editor::ContextManager
#endif // !D_EDITOR_CONTEXT

using namespace D_CORE;
using namespace D_SCENE;

namespace Darius::Editor::ContextManager
{
	void Initialize(Path projectPath);
	void Shutdown();


	GameObject*						GetSelectedGameObject();
	void							SetSelectedGameObject(GameObject* go);

	// Paths
	Path							GetProjectPath();
	Path							GetAssetsPath();
	Path							GetEditorConfigPath();
}
