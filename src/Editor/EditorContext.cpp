#include "Editor/pch.hpp"
#include "EditorContext.hpp"
#include "Gui/GuiManager.hpp"

#include <ResourceManager/ResourceLoader.hpp>
#include <Utils/Assert.hpp>

using namespace D_FILE;

namespace Darius::Editor::ContextManager
{
	bool						_initialized = false;

	GameObject*					SelectedGameObject;
	Detailed*					SelectedDetailed;
	D_FILE::Path				ProjectPath;

	void Initialize(D_FILE::Path projectPath)
	{
		D_ASSERT(!_initialized);
		_initialized = true;
		SelectedGameObject = nullptr;

		ProjectPath = projectPath;

		D_ASSERT_M(D_H_ENSURE_DIR(projectPath), "Project directory is not a valid directory");

		if (!D_H_ENSURE_PATH(GetAssetsPath()))
		{
			D_ASSERT_M(std::filesystem::create_directory(GetAssetsPath()), "Could not detect/create assets directory for project");
		}

		if (!D_H_ENSURE_PATH(GetEditorConfigPath()))
		{
			D_ASSERT_M(std::filesystem::create_directory(GetEditorConfigPath()), "Could not detect/create editor config directory for project");
		}

		D_RESOURCE_LOADER::VisitSubdirectory(GetAssetsPath(), true);

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
		SelectedDetailed = dynamic_cast<Detailed*>(go);
	}

	Detailed* GetSelectedDetailed()
	{
		return SelectedDetailed;
	}

	void SetSelectedDetailed(Detailed* d)
	{
		if (auto go = dynamic_cast<GameObject*>(d); go)
		{
			SetSelectedGameObject(go);
			return;
		}
		SelectedDetailed = d;
		SelectedGameObject = nullptr;
	}

	Path GetProjectPath()
	{
		return ProjectPath;
	}

	Path GetAssetsPath()
	{
		return Path(ProjectPath).append("Assets/");
	}

	Path GetEditorConfigPath()
	{
		return Path(ProjectPath).append("Config/");
	}
}
