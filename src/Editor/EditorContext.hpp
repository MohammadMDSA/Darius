#pragma once

#include <Scene/GameObject.hpp>

#include <filesystem>

#ifndef D_EDITOR_CONTEXT
#define D_EDITOR_CONTEXT Darius::Editor::ContextManager
#endif // !D_EDITOR_CONTEXT

using namespace D_SCENE;

namespace Darius::Editor::ContextManager
{
	void Initialize(std::filesystem::path projectPath);
	void Shutdown();


	GameObject*						GetSelectedGameObject();
	void							SetSelectedGameObject(GameObject* go);
}
