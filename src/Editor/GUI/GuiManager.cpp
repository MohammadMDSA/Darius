#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>

#include "GuiManager.hpp"
#include "SceneWindow.hpp"
#include "SceneGraphWindow.hpp"
#include "DetailsWindow.hpp"
#include "ResourceMonitorWindow.hpp"
#include "ProfilerWindow.hpp"
#include "Editor/EditorContext.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Math/VectorMath.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/ResourceLoader.hpp>
#include <Scene/EntityComponentSystem/Components/TransformComponent.hpp>
#include <Utils/Assert.hpp>

#include <ImGuiFileDialog/ImGuiFileDialog.h>
#include <Libs/FontIcon/IconsFontAwesome5.h>
#include <imgui.h>

using namespace Darius::Editor::Gui::Windows;

namespace Darius::Editor::Gui::GuiManager
{
	bool										initialzied = false;

	D_CONTAINERS::DMap<std::string, Window*>	Windows;

	std::string									LayoutPath;

	void ShowDialogs();

	void Initialize()
	{
		D_ASSERT(!initialzied);
		initialzied = true;

		// TODO: Use linear allocator to allocate windows
		auto sceneWindow = new SceneWindow();
		Windows[sceneWindow->GetName()] = sceneWindow;

		auto sceneGraphWindow = new SceneGraphWindow();
		Windows[sceneGraphWindow->GetName()] = sceneGraphWindow;

		auto detailsWindow = new DetailsWindow();
		Windows[detailsWindow->GetName()] = detailsWindow;

		auto resourceMonitorWindow = new ResourceMonitorWindow();
		Windows[resourceMonitorWindow->GetName()] = resourceMonitorWindow;

		auto profilerWindow = new ProfilerWindow();
		Windows[profilerWindow->GetName()] = profilerWindow;

		ImGuiIO& io = ImGui::GetIO();
		// Setup docking
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		// Setup fonts
		io.Fonts->AddFontDefault();
		// Merge fontawesom fonts
		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
		ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
		io.Fonts->AddFontFromFileTTF("EditorRes/fonts/" FONT_ICON_FILE_NAME_FAS, 12.0f, &icons_config, icons_ranges);


		// Read layout from file
		LayoutPath = Path(D_EDITOR_CONTEXT::GetEditorConfigPath()).append("layout.ini").string();

		io.IniFilename = LayoutPath.c_str();
		ImGui::GetStyle().FrameRounding = 5.f;
		ImGui::GetStyle().PopupRounding = 5.f;
		ImGui::GetStyle().WindowRounding = 5.f;

	}

	void Shutdown()
	{
		D_ASSERT(initialzied);
	}

	void Update(float deltaTime)
	{

		for (auto& kv : Windows)
			kv.second->Update(deltaTime);
	}

	void Render(D_GRAPHICS::GraphicsContext& context)
	{
		for (auto& kv : Windows)
			kv.second->Render(context);
	}

	void DrawGUI()
	{
		D_ASSERT_M(initialzied, "Gui Manager is not initialized yet!");

		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("Editor GUI Root", (bool*)1, windowFlags);
			ImGui::PopStyleVar(3);

			ImGuiID dockspaceId = ImGui::GetID("EditorDockspace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

#pragma region Toolbar
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					/*if (ImGui::MenuItem("Load"))
					{
						D_RESOURCE_LOADER::LoadResource(L"ff.fbx");
					}*/

					if (ImGui::MenuItem(ICON_FA_SAVE "  Save Project"))
						D_RESOURCE::SaveAll();

					if (ImGui::MenuItem(ICON_FA_FOLDER_MINUS "  Close Scene"))
					{
						D_WORLD::Unload();
						D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
					}

					ImGui::Separator();

					if (ImGui::MenuItem(ICON_FA_SAVE "  Save Scene"))
					{
						D_WORLD::Save("Foo", D_EDITOR_CONTEXT::GetAssetsPath().wstring());
					}

					if (ImGui::MenuItem(ICON_FA_FOLDER "  Load Scene"))
					{

						ImGuiFileDialog::Instance()->OpenDialog("LoadScene", "Choose Scene File", ".dar", D_EDITOR_CONTEXT::GetAssetsPath().string(), 1, nullptr);

					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Scene"))
				{
					if (ImGui::MenuItem("Create Game Object"))
					{
						D_WORLD::CreateGameObject();
					}

					if (ImGui::MenuItem("Delete Game Object", (const char*)0, false, D_EDITOR_CONTEXT::GetSelectedGameObject() != nullptr))
					{
						D_WORLD::DeleteGameObject(D_EDITOR_CONTEXT::GetSelectedGameObject());
						D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Resource"))
				{
					if (ImGui::MenuItem("Create Material"))
					{
						D_RESOURCE::GetManager()->CreateMaterial(D_EDITOR_CONTEXT::GetAssetsPath());
					}

					if (ImGui::MenuItem("Don't"))
					{
						for (size_t i = 0; i < 1000; i++)
						{
							D_WORLD::CreateGameObject();
						}

					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Window"))
				{
					for (auto& kv : Windows)
					{
						if (ImGui::MenuItem(kv.second->GetName().c_str()))
						{
							kv.second->mOpen = true;
						}
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}
			ShowDialogs();
#pragma endregion

			ImGui::End();
		}

		{
			static bool show_demo_window = true;
			if (show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);

		}

		for (auto& kv : Windows)
		{
			auto wind = kv.second;

			ImGui::SetNextWindowBgAlpha(1.f);

			if (wind->mOpen)
			{
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(wind->mPadding[0], wind->mPadding[1]));
				ImGui::Begin(wind->GetName().c_str(), &wind->mOpen);
				wind->PrepareGUI();

				wind->DrawGUI();

				ImGui::End();
				ImGui::PopStyleVar();

			}
		}
	}

	void ShowDialogs()
	{
		if (ImGuiFileDialog::Instance()->Display("LoadScene"))
		{
			// action if OK
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				if (D_WORLD::Load(WSTR_STR(filePathName)))
					D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
			}

			// close
			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("SaveScene"))
		{
			// action if OK
			if (ImGuiFileDialog::Instance()->IsOk())
			{
				std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				if (D_WORLD::Load(WSTR_STR(filePathName)))
					D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
			}

			// close
			ImGuiFileDialog::Instance()->Close();
		}
	}
}