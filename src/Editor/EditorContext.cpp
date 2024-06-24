#include "Editor/pch.hpp"
#include "EditorContext.hpp"

#include "GUI/DetailDrawer/DetailDrawer.hpp"
#include "GUI/GuiManager.hpp"
#include "GUI/GuiRenderer.hpp"
#include "GUI/ThumbnailManager.hpp"
#include "Simulation.hpp"

#include <Core/Serialization/Copyable.hpp>
#include <Engine/EngineContext.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>
#include <ResourceManager/ResourceLoader.hpp>
#include <Scene/Scene.hpp>
#include <Utils/Assert.hpp>

using namespace D_CORE;
using namespace D_FILE;
using namespace D_SCENE;
using namespace D_SERIALIZATION;

namespace Darius::Editor::Context
{
	bool						_initialized = false;

	GameObject*					SelectedGameObject;
	Detailed*					SelectedDetailed;
	ICopyable*					Clipboard;


	void Initialize(HWND wind)
	{
		D_ASSERT(!_initialized);
		_initialized = true;
		SelectedGameObject = nullptr;

		if (!D_H_ENSURE_PATH(GetEditorConfigPath()))
		{
			D_ASSERT_M(std::filesystem::create_directory(GetEditorConfigPath()), "Could not detect/create editor config directory for project");
		}

		D_GUI_RENDERER::Initialize();

		auto directoryVisitProgress = new D_RESOURCE::DirectoryVisitProgress();
		directoryVisitProgress->OnFinish = []()
			{
				D_WORLD::LoadPrefabs();
			};
		directoryVisitProgress->Deletable.store(true);
		D_RESOURCE_LOADER::VisitSubdirectory(D_ENGINE_CONTEXT::GetAssetsPath(), true, directoryVisitProgress);

		D_THUMBNAIL::Initialize();
		D_GUI_MANAGER::Initialize(wind);

		// Initializing the simulator
		D_SIMULATE::Initialize();

		D_DETAIL_DRAWER::Initialize();

	}

	void Shutdown()
	{
		D_ASSERT(_initialized);

		D_DETAIL_DRAWER::Shutdown();
		D_SIMULATE::Shutdown();
		D_GUI_MANAGER::Shutdown();
		D_THUMBNAIL::Shutdown();
		D_GUI_RENDERER::Shutdown();
	}

	void Update(float elapsedTime)
	{

		// Scene Initialization for frame
		{
			D_PROFILING::ScopedTimer _prof(L"Scene Initialization");
			D_WORLD::FrameInitialization();
		}

		// Updating the simulator
		D_SIMULATE::Update(elapsedTime, [elapsedTime]()
			{
				D_GUI_MANAGER::Update(elapsedTime);
			});

	}

	bool VerifyClipboard()
	{
		if (Clipboard && !Clipboard->IsCopyableValid())
			Clipboard = nullptr;

		return Clipboard != nullptr;
	}

	void SetClipboard(ICopyable* copyable)
	{
		Clipboard = copyable;

		VerifyClipboard();
	}

	void GetClipboardJson(bool maintainContext, D_SERIALIZATION::Json& result)
	{
		if (VerifyClipboard())
		{
			Clipboard->Copy(maintainContext, result);
		}
		else
		{
			result = Json();
		}
		
	}

	bool IsGameObjectInClipboard()
	{
		VerifyClipboard();
		return dynamic_cast<GameObject*>(Clipboard);
	}

	GameObject* GetSelectedGameObject()
	{
		D_ASSERT(_initialized);

		if (SelectedGameObject && !SelectedGameObject->IsValid())
			SetSelectedGameObject(nullptr);

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
}
