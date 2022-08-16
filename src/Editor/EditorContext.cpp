#include "Editor/pch.hpp"
#include "EditorContext.hpp"
#include "Gui/GuiManager.hpp"

#include <Utils/Assert.hpp>

namespace Darius::Editor::ContextManager
{
	bool						_initialized = false;

	GameObject*					SelectedGameObject;
	D_CORE::Path				ProjectPath;

	void Initialize(D_CORE::Path projectPath)
	{
		D_ASSERT(!_initialized);
		_initialized = true;
		SelectedGameObject = nullptr;

		ProjectPath = projectPath;

		D_ASSERT_M(std::filesystem::exists(projectPath) && std::filesystem::is_directory(projectPath), "Project directory is not a valid directory");

		D_GUI_MANAGER::Initialize();
	}

	void Shutdown()
	{
		D_ASSERT(_initialized);
		D_GUI_MANAGER::Shutdown();
	}

	GameObject* GetSelectedGameObject()
	{
		D_ASSERT(_initialized);
		return SelectedGameObject;
	}

	void SetSelectedGameObject(GameObject* go)
	{
		D_ASSERT(_initialized);
		SelectedGameObject = go;
	}
}
