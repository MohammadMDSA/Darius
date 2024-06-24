#pragma once

#include <Core/Filesystem/Path.hpp>
#include <Scene/GameObject.hpp>
#include <Utils/Detailed.hpp>

#ifndef D_EDITOR_CONTEXT
#define D_EDITOR_CONTEXT Darius::Editor::Context
#endif // !D_EDITOR_CONTEXT

namespace Darius::Editor::Context
{
	void Initialize(HWND wind);
	void Shutdown();
	void Update(float elapsedTime);

	D_SCENE::GameObject*			GetSelectedGameObject();
	void							SetSelectedGameObject(D_SCENE::GameObject* go);
	Detailed*						GetSelectedDetailed();
	void							SetSelectedDetailed(Detailed* d);

	void							SetClipboard(class D_SERIALIZATION::ICopyable* copyable);
	void							GetClipboardJson(bool maintainContext, D_SERIALIZATION::Json& result);
	bool							IsGameObjectInClipboard();

	// Paths
	D_FILE::Path					GetProjectPath();
	D_FILE::Path					GetEditorConfigPath();
	D_FILE::Path					GetEditorWindowsConfigPath();
}
