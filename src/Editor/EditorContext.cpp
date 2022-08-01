#include "Editor/pch.hpp"
#include "EditorContext.hpp"
#include "Gui/GuiManager.hpp"

#include <Utils/Assert.hpp>

namespace Darius::Editor::ContextManager
{
	bool						_initialized = false;

	GameObject*					SelectedGameObject;

	void Initialize()
	{
		D_ASSERT(!_initialized);
		_initialized = true;
		SelectedGameObject = nullptr;
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
