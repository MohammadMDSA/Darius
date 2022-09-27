#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>

#include "GuiManager.hpp"
#include "SceneWindow.hpp"
#include "SceneGraphWindow.hpp"
#include "DetailsWindow.hpp"
#include "ResourceMonitorWindow.hpp"
#include "ProfilerWindow.hpp"
#include "Editor/EditorContext.hpp"
#include "Editor/Simulation.hpp"

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
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <imgui.h>
#include <imgui_impl_dx12.h>
#include <imgui_impl_win32.h>

using namespace Darius::Editor::Gui::Windows;

namespace Darius::Editor::Gui::GuiManager
{
	bool										initialzied = false;

	D_CONTAINERS::DMap<std::string, Window*>	Windows;

	std::string									LayoutPath;

	void ShowDialogs();

	void RootToolbar();

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
		io.Fonts->AddFontFromFileTTF("EditorResources/fonts/" FONT_ICON_FILE_NAME_FAS, 12.0f, &icons_config, icons_ranges);


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

		// Prepare imgui
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		DrawGUI();

		ImGui::Render();
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

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.05f, 0.1f, 1.f));
			ImGui::Begin("Editor GUI Root", (bool*)1, windowFlags);
			ImGui::PopStyleColor();
			RootToolbar();
			ImGui::PopStyleVar(3);

			ImGuiID dockspaceId = ImGui::GetID("EditorDockspace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

#pragma region Toolbar
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu(ICON_FA_FILE "  File"))
				{
					/*if (ImGui::MenuItem("Load"))
					{
						D_RESOURCE_LOADER::LoadResource(L"ff.fbx");
					}*/

					if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK "  Save Project"))
						D_RESOURCE::SaveAll();

					ImGui::Separator();

					auto simulating = D_SIMULATE::IsSimulating();

					if (simulating)
						ImGui::BeginDisabled();

					if (ImGui::MenuItem(ICON_FA_XMARK"  Close Scene"))
					{
						D_WORLD::Unload();
						D_EDITOR_CONTEXT::SetSelectedGameObject(nullptr);
					}

					if (ImGui::MenuItem(ICON_FA_FLOPPY_DISK "  Save Scene"))
					{
						if (D_WORLD::IsLoaded())
							D_WORLD::Save();
						else
							ImGuiFileDialog::Instance()->OpenDialog("SaveScene", "Create Scene File", ".dar", D_EDITOR_CONTEXT::GetAssetsPath().string());
					}

					if (ImGui::MenuItem(ICON_FA_FOLDER "  Load Scene"))
					{

						ImGuiFileDialog::Instance()->OpenDialog("LoadScene", "Choose Scene File", ".dar", D_EDITOR_CONTEXT::GetAssetsPath().string(), 1, nullptr);

					}

					if (simulating)
						ImGui::EndDisabled();

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
				if (D_WORLD::Create(WSTR_STR(filePathName)))
				{
					D_WORLD::Save();
				}
			}

			// close
			ImGuiFileDialog::Instance()->Close();
		}
	}

	void RootToolbar()
	{
		auto width = ImGui::GetWindowWidth();
		auto buttonSize = 30.f;
		auto margin = 5.f;

		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(margin, margin));

		auto offset = (width - 3 * buttonSize) / 2;

		auto isSimulating = D_SIMULATE::IsSimulating();

		// Play button
		ImGui::SameLine(offset);
		if (ImGui::Button(isSimulating ? ICON_FA_STOP : ICON_FA_PLAY, ImVec2(buttonSize, 0)))
			isSimulating ? D_SIMULATE::Stop() : D_SIMULATE::Run();

		// Update states
		isSimulating = D_SIMULATE::IsSimulating();
		auto isPaused = D_SIMULATE::IsPaused();



		// Pause button
		// Deciding puase button color
		if (isPaused)
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.26f, 0.59f, 1.f, 1.f });

		ImGui::SameLine();

		// Disable if not simulating
		if (!isSimulating)
			ImGui::BeginDisabled();

		// The button itself
		if (ImGui::Button(ICON_FA_PAUSE, ImVec2(buttonSize, 0)))
			isPaused ? D_SIMULATE::Resume() : D_SIMULATE::Pause();

		// Cleaning up disabled and color status if necessary
		if (!isSimulating)
			ImGui::EndDisabled();
		if (isPaused)
			ImGui::PopStyleColor();

		isPaused = D_SIMULATE::IsPaused();

		// Step button
		ImGui::SameLine();

		if (!isPaused)
			ImGui::BeginDisabled();
		if (ImGui::Button(ICON_FA_FORWARD_STEP, ImVec2(buttonSize, 0)))
			D_SIMULATE::Step();
		if (!isPaused)
			ImGui::EndDisabled();

		ImGui::PopStyleVar();
	}
}