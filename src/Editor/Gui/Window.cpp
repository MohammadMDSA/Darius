#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "Window.hpp"

#include <imgui.h>

namespace Darius::Editor::Gui::Windows
{

	Window::Window():
		mWidth(1.f),
		mHeight(1.f),
		mPosX(0.f),
		mPosY(0.f),
		mHovered(false)
	{
	}

	void Window::PrepareGUI()
	{
		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();
		mWidth = XMMax(max.x - min.x, 1.f);
		mHeight = XMMax(max.y - min.y, 1.f);
		auto pos = ImGui::GetWindowPos();
		mPosX = pos.x;
		mPosY = pos.y;

		mHovered = ImGui::IsWindowHovered();

	}
}