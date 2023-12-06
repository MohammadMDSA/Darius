#pragma once

#include "Window.hpp"

#include "SettingsWindow.generated.hpp"

namespace Darius::Editor::Gui::Windows
{
	class DClass(Serialize) SettingsWindow : public Window
	{
		GENERATED_BODY();
		D_CH_EDITOR_WINDOW_BODY(SettingsWindow, "Settings");

	public:

		// Inherited via Window
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


	private:

		void DrawNavigator();
		void DrawRightPanel();

		D_CONTAINERS::DMap<std::string, std::function<bool(D_SERIALIZATION::Json&)>> mSubsystemsSettings;

		DField(Get[inline, const, &])		
		std::string					mSelectedCategory;

		DField(Get[inline, const, &])		
		D_SERIALIZATION::Json		mSettings;

		float						mNavigatorWidth;
		float						mRightPanelWidth;
	};
}

File_SettingsWindow_GENERATED