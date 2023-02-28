#pragma once

#include <Core/Filesystem/Path.hpp>
#include <Core/Signal.hpp>
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
	Path							GetEditorWindowsConfigPath();

	// Events
	D_H_SIGNAL_DECL(EditorSuspended, void());
	D_H_SIGNAL_DECL(EditorResuming, void());
	D_H_SIGNAL_DECL(EditorDeactivated, void());
	D_H_SIGNAL_DECL(EditorActivated, void());
	D_H_SIGNAL_DECL(EditorQuitting, void());

	// Internal
	void							EditorSuspended();
	void							EditorResuming();
	void							EditorDeactivated();
	void							EditorActivated();
	void							EditorQuitting();
}
