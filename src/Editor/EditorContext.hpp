#pragma once

#include <Scene/GameObject.hpp>

#ifndef D_EDITOR_CONTEXT
#define D_EDITOR_CONTEXT Darius::Editor::ContextManager
#endif // !D_EDITOR_CONTEXT

using namespace D_SCENE;

namespace Darius::Editor::ContextManager
{
	void Initialize();
	void Shutdown();


	GameObject*						GetSelectedGameObject();
	void							SetSelectedGameObject(GameObject* go);
}
