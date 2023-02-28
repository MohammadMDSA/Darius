#include "Editor/pch.hpp"
#include "EditorContext.hpp"

#include "GUI/GuiManager.hpp"
#include "GUI/ThumbnailManager.hpp"
#include "Simulation.hpp"

#include <Engine/EngineContext.hpp>
#include <ResourceManager/ResourceLoader.hpp>
#include <Utils/Assert.hpp>

using namespace D_FILE;

namespace Darius::Editor::Context
{
	bool						_initialized = false;

	GameObject*					SelectedGameObject;
	Detailed*					SelectedDetailed;

	D_H_SIGNAL_DEFINITION(EditorSuspended, void());
	D_H_SIGNAL_DEFINITION(EditorResuming, void());
	D_H_SIGNAL_DEFINITION(EditorDeactivated, void());
	D_H_SIGNAL_DEFINITION(EditorActivated, void());
	D_H_SIGNAL_DEFINITION(EditorQuitting, void());


	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;
		SelectedGameObject = nullptr;

		if (!D_H_ENSURE_PATH(GetEditorConfigPath()))
		{
			D_ASSERT_M(std::filesystem::create_directory(GetEditorConfigPath()), "Could not detect/create editor config directory for project");
		}

		D_RESOURCE_LOADER::VisitSubdirectory(D_ENGINE_CONTEXT::GetAssetsPath(), true);

		D_THUMBNAIL::Initialize();
		D_GUI_MANAGER::Initialize();

		// Initializing the simulator
		D_SIMULATE::Initialize();

	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		D_SIMULATE::Shutdown();
		D_GUI_MANAGER::Shutdown();
		D_THUMBNAIL::Shutdown();
	}

	void Update(float elapsedTime)
	{

		D_GUI_MANAGER::Update(elapsedTime);

		// Updating the simulator
		D_SIMULATE::Update();

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
		return D_ENGINE_CONTEXT::GetProjectPath();
	}
	
	Path GetEditorConfigPath()
	{
		return D_ENGINE_CONTEXT::GetProjectPath().append("Config/Editor/");
	}

	Path GetEditorWindowsConfigPath()
	{
		return GetEditorConfigPath() / "Windows.json";
	}

	void EditorSuspended()
	{
		EditorSuspendedSignal();
	}

	void EditorResuming()
	{
		EditorResumingSignal();
	}

	void EditorDeactivated()
	{
		EditorDeactivatedSignal();
	}

	void EditorActivated()
	{
		EditorActivatedSignal();
	}

	void EditorQuitting()
	{
		EditorQuittingSignal();
	}
}
