#pragma once

#include "Window.hpp"

namespace Darius::Editor::Gui::Windows
{
	class SequencerWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY_RAW(SequencerWindow, "Sequencer");

	public:
		SequencerWindow(D_SERIALIZATION::Json const& config);
		~SequencerWindow() = default;

		SequencerWindow(SequencerWindow const& other) = delete;

		virtual void DrawGUI() override;
	};
}