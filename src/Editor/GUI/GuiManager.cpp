#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "GuiManager.hpp"
#include "SceneWindow.hpp"
#include "SceneGraphiWindow.hpp"

#include <Core/Containers/Map.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Math/VectorMath.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Utils/Assert.hpp>

#include <imgui/imgui.h>

using namespace Darius::Editor::Gui::Windows;

namespace Darius::Editor::GuiManager
{
	bool										initialzied = false;

	D_CONTAINERS::DMap<std::string, Window*>		Windows;

	D_SCENE::GameObject* ri;
	

	void Initialize()
	{
		D_ASSERT(!initialzied);
		initialzied = true;

		// TODO: Use linear allocator to allocate windows
		auto sceneWindow = new SceneWindow();
		Windows[sceneWindow->GetName()] = sceneWindow;

		auto sceneGraphWindow = new SceneGraphWindow();
		Windows[sceneGraphWindow->GetName()] = sceneGraphWindow;

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("Editor GUI Root", (bool*) 1, windowFlags);
			ImGui::PopStyleVar(3);

			ImGuiID dockspaceId = ImGui::GetID("EditorDockspace");
			ImGui::DockSpace(dockspaceId, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

			ImGui::End();
		}

		{
			static bool show_demo_window = true;
			static bool show_another_window = true;
			static float clear_color[] = { 1.f, 1.f, 1.f, 0.f };
			// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
			if (show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);

			// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
			{
				static float f = 0.0f;
				static int counter = 0;

				ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

				ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
				ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
				ImGui::Checkbox("Another Window", &show_another_window);

				ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
				ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

				if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
					counter++;
				ImGui::SameLine();
				ImGui::Text("counter = %d", counter);

				ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}

			// 3. Show another simple window.
			if (show_another_window)
			{
				ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				ImGui::Text("Hello from another window!");
				if (ImGui::Button("Close Me"))
					show_another_window = false;
				ImGui::End();
			}
		}

		for (auto& kv : Windows)
		{
			auto wind = kv.second;

			ImGui::Begin(wind->GetName().c_str());
			wind->PrepareGUI();

			wind->DrawGUI();

			ImGui::End();
		}
	}

}