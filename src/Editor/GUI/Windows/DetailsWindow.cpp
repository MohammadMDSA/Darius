#include "Editor/pch.hpp"
#include "DetailsWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <ResourceManager/ResourceLoader.hpp>

using namespace D_RESOURCE;

namespace Darius::Editor::Gui::Windows
{

	DetailsWindow::DetailsWindow(D_SERIALIZATION::Json const& config) :
		Window(config)
	{
	}

	DetailsWindow::~DetailsWindow()
	{
	}

	void DetailsWindow::DrawGUI()
	{
		D_PROFILING::ScopedTimer profiling(L"Details Window Draw GUI");
		auto obj = D_EDITOR_CONTEXT::GetSelectedDetailed();
		if (!obj)
			return;

		if (auto resource = dynamic_cast<Resource*>(obj); resource)
		{
			if (!resource->IsLoaded())
			{
				D_RESOURCE_LOADER::LoadResource(resource);
				return;
			}
		}

		obj->DrawDetails(nullptr);
	}
}
