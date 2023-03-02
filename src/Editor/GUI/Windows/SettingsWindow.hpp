#pragma once

#include "Window.hpp"

namespace Darius::Editor::Gui::Windows
{
	class SettingsWindow : public Window
	{
		D_CH_EDITOR_WINDOW_BODY(SettingsWindow, "Settings");

	public:
		SettingsWindow(D_SERIALIZATION::Json const& config);
		~SettingsWindow();

		// Inherited via Window
		INLINE virtual void			Render(D_GRAPHICS::GraphicsContext&) override {}

		INLINE virtual void			Update(float) override {}

		virtual void				DrawGUI() override;

		void						SaveSettings() const;
		void						LoadSettings();

		INLINE void					SetSelectedCategory(std::string const& val)
		{
			if (!mSubsystemsSettings.contains(val))
				return;
			mSelectedCategory = val;
		}

		D_CH_R_FIELD(std::string, SelectedCategory);
		D_CH_R_FIELD(D_SERIALIZATION::Json, Settings);


	private:

		void DrawNavigator();
		void DrawRightPanel();

		D_CONTAINERS::DMap<std::string, std::function<bool(D_SERIALIZATION::Json&)>> mSubsystemsSettings;

		float						mNavigatorWidth;
		float						mRightPanelWidth;
	};
}
