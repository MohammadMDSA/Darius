#pragma once

#include <Core/Filesystem/Path.hpp>
#include <Scene/GameObject.hpp>

#include <Utils/Detailed.hpp>

#ifndef D_EDITOR_CONTEXT
#define D_EDITOR_CONTEXT Darius::Editor::ContextManager
#endif // !D_EDITOR_CONTEXT

using namespace D_FILE;
using namespace D_SCENE;

namespace Darius::Editor::ContextManager
{
	void Initialize(Path projectPath);
	void Shutdown();


	GameObject*						GetSelectedGameObject();
	void							SetSelectedGameObject(GameObject* go);
	Detailed*						GetSelectedDetailed();
	void							SetSelectedDetailed(Detailed* d);

	// Paths
	Path							GetProjectPath();
	Path							GetAssetsPath();
	Path							GetEditorConfigPath();
}
