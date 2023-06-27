#pragma once

#include <Graphics/CommandContext.hpp>
#include <Graphics/GraphicsUtils/Profiling/Profiling.hpp>

#include "Window.generated.hpp"

namespace Darius::Editor::Gui::GuiManager
{
	void DrawGUI();
	void _DrawMenuBar();
}

#define D_CH_EDITOR_WINDOW_BODY_RAW(type, name) \
public: \
static INLINE std::string SGetName() { return name; } \
INLINE virtual std::string GetName() const { return SGetName(); }

#define D_CH_EDITOR_WINDOW_BODY(type, name) \
D_CH_EDITOR_WINDOW_BODY_RAW(type, name) \
public: \
type(D_SERIALIZATION::Json const& config); \
~type();

namespace Darius::Editor::Gui::Windows
{
	class DClass(Serialize) Window
	{
	public:
		Window(D_SERIALIZATION::Json const& config);
		~Window() = default;

		Window(Window const& other) = delete;

		virtual std::string 		GetName() const = 0;

		virtual INLINE void			Render() {};
		virtual void				Update(float dt) = 0;
		virtual void				DrawGUI() = 0;
		void						PrepareGUI();


	protected:

		friend void Darius::Editor::Gui::GuiManager::DrawGUI();
		friend void Darius::Editor::Gui::GuiManager::_DrawMenuBar();

		DField(Get[inline], Set[inline])
		bool						mOpened;

		float						mPadding[2] = { 8.f, 8.f };

		float						mWidth;
		float						mHeight;

		float						mPosX;
		float						mPosY;

		bool						mHovered;
		bool						mFocused;

	public:
		Darius_Editor_Gui_Windows_Window_GENERATED
	};

}

File_Window_GENERATED