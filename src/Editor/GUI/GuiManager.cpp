#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "GuiManager.hpp"
#include "Editor/Camera.hpp"

#include <Core/Input.hpp>
#include <Core/TimeManager/TimeManager.hpp>
#include <Math/VectorMath.hpp>
#include <Renderer/Renderer.hpp>
#include <Renderer/Camera/CameraManager.hpp>
#include <Utils/Assert.hpp>

#include "imgui/imgui.h"
#include "ImGuizmo/ImGuizmo.h"

namespace Darius::Editor::GuiManager
{
	bool										initialzied = false;
	std::unique_ptr<D_MATH_CAMERA::Camera>      Cam;

	float										Width, Height;
	float										posX, posY;
	bool										IsHovered;

	D_GRAPHICS_BUFFERS::ColorBuffer				SceneTexture;
	D_GRAPHICS_BUFFERS::DepthBuffer				SceneDepth;
	DescriptorHandle							TextureHandle;

	RenderItem* ri;

	void CreateBuffers();
	D_RENDERER_FRAME_RESOUCE::GlobalConstants GetGlobalConstants();

	void Initialize()
	{
		D_ASSERT(!initialzied);
		initialzied = true;

		CreateBuffers();
		TextureHandle = D_RENDERER::GetRenderResourceHandle(1);

		Cam = std::make_unique<D_MATH_CAMERA::Camera>();
		Cam->SetFOV(XM_PI / 3);
		Cam->SetZRange(0.01f, 1000.f);
		Cam->SetPosition(Vector3(-5.f, 0.f, 0));
		Cam->SetLookDirection(Vector3::Right(), Vector3::Up());
		Cam->ReverseZ(false);

		D_CAMERA_MANAGER::SetActiveCamera(Cam.get());
		Width = Height = 1;
		posX = posY = 0;
		IsHovered = false;

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	}

	void Shutdown()
	{
		D_ASSERT(initialzied);
		Cam.reset();
		SceneDepth.Destroy();
		SceneTexture.Destroy();
	}

	void Update(float deltaTime)
	{
		if (D_CAMERA_MANAGER::SetViewportDimansion(Width, Height))
		{
			CreateBuffers();
		}

		static auto fc = FlyingFPSCamera(*Cam, Vector3::Up());

		if (D_MOUSE::GetButton(D_MOUSE::Keys::Right) && IsHovered)
			fc.Update(deltaTime);
		else
			Cam->Update();
	}

	void Render(D_GRAPHICS::GraphicsContext& context, std::vector<RenderItem*> const& renderItems)
	{
		context.SetPipelineState(D_RENDERER::Psos["opaque"]);

		// Prepare the command list to render a new frame.
		context.TransitionResource(SceneTexture, D3D12_RESOURCE_STATE_RENDER_TARGET, true);

		PIXBeginEvent(PIX_COLOR_DEFAULT, "Clear Scene Buffers");

		// Clear the views.
		auto const rtvDescriptor = SceneTexture.GetRTV();
		auto const dsvDescriptor = SceneDepth.GetDSV();

		// Set the viewport and scissor rect.
		auto viewport = CD3DX12_VIEWPORT(0.f, 0.f, Width, Height, D3D12_MIN_DEPTH, D3D12_MAX_DEPTH);
		auto scissorRect = CD3DX12_RECT(0l, 0l, (long)(Width), (long)(Height));

		context.ClearColor(SceneTexture, &scissorRect);
		context.ClearDepth(SceneDepth);
		context.SetRenderTarget(rtvDescriptor, dsvDescriptor);
		context.SetViewportAndScissor(viewport, scissorRect);

		PIXEndEvent();

		auto globals = GetGlobalConstants();
		D_RENDERER::RenderMeshes(context, renderItems, globals);

		context.TransitionResource(SceneTexture, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, true);
		D_RENDERER_DEVICE::GetDevice()->CopyDescriptorsSimple(1, TextureHandle, SceneTexture.GetSRV(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	void DrawGUI()
	{
		D_ASSERT_M(initialzied, "Gui Manager is not initialized yet!");

		{
			ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
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


		ImGui::Begin("Scene");

		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();
		Width = max.x - min.x;
		Height = max.y - min.y;
		auto pos = ImGui::GetWindowPos();
		posX = pos.x;
		posY = pos.y;

		IsHovered = ImGui::IsWindowHovered();

		ImGui::Image((ImTextureID)TextureHandle.GetGpuPtr(), ImVec2(Width, Height));


		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(0.f, 0.f, Width, Height);
		auto view = Cam->GetViewMatrix();
		auto proj = Cam->GetProjMatrix();
		float* world = (float*)&ri->World;
		if(ImGuizmo::Manipulate((float*)&view, (float*)&proj, ImGuizmo::OPERATION::TRANSLATE, ImGuizmo::MODE::LOCAL, world))
			ri->World = D_MATH::Matrix4(world);

		ImGui::End();

	}

	void CreateBuffers()
	{
		SceneTexture.Create(L"Scene Texture", (UINT)Width, (UINT)Height, 1, D_RENDERER_DEVICE::GetBackBufferFormat());
		SceneDepth.Create(L"Scene DepthStencil", (UINT)Width, (UINT)Height, D_RENDERER_DEVICE::GetDepthBufferFormat());
	}

	D_RENDERER_FRAME_RESOUCE::GlobalConstants GetGlobalConstants()
	{
		D_RENDERER_FRAME_RESOUCE::GlobalConstants globals;

		auto camera = Cam.get();

		auto view = camera->GetViewMatrix();
		auto proj = camera->GetProjMatrix();

		auto viewProj = camera->GetViewProjMatrix();
		auto invView = Matrix4::Inverse(view);
		auto invProj = Matrix4::Inverse(proj);
		auto invViewProj = Matrix4::Inverse(viewProj);

		float width, height;
		D_CAMERA_MANAGER::GetViewportDimansion(width, height);

		auto time = *D_TIME::GetStepTimer();

		globals.View = view;
		globals.InvView = invView;
		globals.Proj = proj;
		globals.InvProj = invProj;
		globals.ViewProj = viewProj;
		globals.InvViewProj = invViewProj;
		globals.CameraPos = camera->GetPosition();
		globals.RenderTargetSize = XMFLOAT2(width, height);
		globals.InvRenderTargetSize = XMFLOAT2(1.f / width, 1.f / height);
		globals.NearZ = camera->GetNearClip();
		globals.FarZ = camera->GetFarClip();
		globals.TotalTime = (float)time.GetTotalSeconds();
		globals.DeltaTime = (float)time.GetElapsedSeconds();

		return globals;
	}
}