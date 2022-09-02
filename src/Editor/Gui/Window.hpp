#pragma once

#include <Renderer/CommandContext.hpp>

using namespace D_GRAPHICS;

namespace Darius::Editor::Gui::GuiManager
{
	void DrawGUI();
}

namespace Darius::Editor::Gui::Windows
{
	class Window
	{
	public:
		Window();
		~Window() = default;

		Window(Window const& other) = delete;

		virtual std::string const	GetName() = 0;

		virtual void				Render(D_GRAPHICS::GraphicsContext& context) = 0;
		virtual void				Update(float dt) = 0;
		virtual void				DrawGUI() = 0;
		void						PrepareGUI();

	protected:

		friend void Darius::Editor::Gui::GuiManager::DrawGUI();

		float						mPadding[2] = { 8.f, 8.f };

		float						mWidth;
		float						mHeight;

		float						mPosX;
		float						mPosY;

		bool						mOpen = true;
		bool						mHovered;
		bool						mFocused;
	};

}