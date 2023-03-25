#include "Editor/pch.hpp"
#include <Renderer/pch.hpp>
#include "Window.hpp"

#include <imgui.h>

#include "Window.sgenerated.hpp"

namespace Darius::Editor::Gui::Windows
{

	Window::Window(D_SERIALIZATION::Json const& settings):
		mWidth(1.f),
		mHeight(1.f),
		mPosX(0.f),
		mPosY(0.f),
		mHovered(false),
		mFocused(false)
	{
		D_H_OPTIONS_LOAD_BASIC_DEFAULT("Opened", mOpened, false);
	}

	void Window::PrepareGUI()
	{
		auto min = ImGui::GetWindowContentRegionMin();
		auto max = ImGui::GetWindowContentRegionMax();
		mWidth = std::max(max.x - min.x, 1.f);
		mHeight = std::max(max.y - min.y, 1.f);
		auto pos = ImGui::GetWindowPos();
		mPosX = pos.x;
		mPosY = pos.y;

		mFocused = ImGui::IsWindowFocused();
		mHovered = ImGui::IsWindowHovered();

	}
}