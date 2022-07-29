#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "GuiManager.hpp"
#include "Editor/Camera.hpp"

#include <Core/Input.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Utils/Assert.hpp>

#include "imgui/imgui.h"

namespace Darius::Editor::GuiManager
{
	bool										initialzied = false;
	std::unique_ptr<D_MATH_CAMERA::Camera>      Camera;

	float										Width, Height;
	float										posX, posY;

	void Initialize()
	{
		D_ASSERT(!initialzied);
		initialzied = true;

		Camera = std::make_unique<D_MATH_CAMERA::Camera>();
		Camera->SetFOV(XM_PI / 3);
		Camera->SetZRange(0.01f, 1000.f);
		Camera->SetPosition(Vector3(-5.f, 0.f, 0));
		Camera->SetLookDirection(Vector3::Right(), Vector3::Up());
		Camera->ReverseZ(false);

		D_CAMERA_MANAGER::SetActiveCamera(Camera.get());
		Width = Height = 1;
		posX = posY = 0;
	}

	void Shutdown()
	{
		D_ASSERT(initialzied);
		Camera.reset();
	}

	void Update(float deltaTime)
	{
		D_CAMERA_MANAGER::SetViewportDimansion(Width, Height);
		D_RENDERER::SetRendererDimansions(Width, Height);

		static auto fc = FlyingFPSCamera(*Camera, Vector3::Up());

		if (D_KEYBOARD::GetKey(D_KEYBOARD::Keys::LeftAlt) &&
			D_MOUSE::GetButton(D_MOUSE::Keys::Right))
			fc.Update(deltaTime);
		else
			Camera->Update();
	}

	void Render(D_GRAPHICS::GraphicsContext& context)
	{
		(context);
	}

	void DrawGUI()
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
		
		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();
		Width = max.x - min.x;
		Height = max.y - min.y;
		auto pos = ImGui::GetWindowPos();
		posX = pos.x;
		posY = pos.y;

		auto id = D_RENDERER::GetSceneTextureHandle();
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::Image((ImTextureID)id.ptr, ImVec2(Width, Height));
		(id);
		ImGui::End();
		ImGui::Render();
	}
}