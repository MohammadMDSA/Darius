#include "Editor/pch.hpp"
#include "GuiManager.hpp"

#include <Renderer/Renderer.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Utils/Assert.hpp>

#include "imgui/imgui.h"

namespace Darius::Editor::GuiManager
{
	bool initialzied = false;

	void RenderSceneToTexture()
	{

	}

	void Initialize()
	{
		D_ASSERT(!initialzied);
		initialzied = true;
	}

	void Shutdown()
	{
		D_ASSERT(initialzied);
	}

	void Render()
	{
		D_ASSERT_M(initialzied, "Gui Manager is not initialized yet!");
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


		ImGui::Begin("Scene");
		
		auto id = D_RENDERER::GetSceneTextureHandle();
		ImGui::SetWindowSize(ImVec2(200, 200));
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::Image((ImTextureID)id.ptr, ImVec2(100.f, 100.f));
		(id);
		ImGui::End();
		ImGui::Render();
	}
}