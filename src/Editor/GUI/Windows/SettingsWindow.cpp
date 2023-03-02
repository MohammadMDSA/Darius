#include "Editor/pch.hpp"
#include "SettingsWindow.hpp"

#include "Editor/GUI/Components/Common.hpp"

#include <Engine/EngineContext.hpp>
#include <Engine/SubsystemRegistry.hpp>

#include <imgui.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

using namespace D_FILE;

namespace Darius::Editor::Gui::Windows
{
	SettingsWindow::SettingsWindow(D_SERIALIZATION::Json const& config) :
		Window(config),
		mNavigatorWidth(-1.f),
		mRightPanelWidth(-1.f)
	{
		D_CONTAINERS::DUnorderedMap<std::string, std::function<bool(D_SERIALIZATION::Json&)>> settingsMap;
		D_SUBSYSTEMS::GetSubsystemsOptionsDrawer(settingsMap);

		for (auto const& keyVal : settingsMap)
		{
			mSubsystemsSettings.insert(keyVal);
		}

		LoadSettings();
	}

	SettingsWindow::~SettingsWindow()
	{
	}

	void SettingsWindow::DrawGUI()
	{
		auto availableWidth = ImGui::GetContentRegionAvail().x;
		auto availableHeigh = ImGui::GetContentRegionAvail().y;

		if (mNavigatorWidth < 0)
			mNavigatorWidth = availableWidth / 5;

		mRightPanelWidth = availableWidth - mNavigatorWidth - 8.f;

		D_GUI_COMPONENT::Splitter(true, 8.f, &mNavigatorWidth, &mRightPanelWidth, 50.f, 50.f, availableHeigh);

		ImGui::BeginChild("##SettingsNavigationPane", ImVec2(mNavigatorWidth, availableHeigh));
		{
			DrawNavigator();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("##SettingsMainPane", ImVec2(mRightPanelWidth, availableHeigh));
		{
			DrawRightPanel();
		}
		ImGui::EndChild();
	}

	void SettingsWindow::DrawNavigator()
	{
		//if (ImGui::BeginListBox("SettingsNavigationList", ImVec2(-FLT_MIN, -FLT_MIN)))
		{
			auto& toBeSelected = mSelectedCategory;

			ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0.f, 0.5f));
			for (auto const& keyVal : mSubsystemsSettings)
			{
				auto const& key = keyVal.first;
				bool selected = mSelectedCategory == key;

				ImGui::SameLine(10);
				if (ImGui::Selectable(key.c_str(), &selected, 0, ImVec2(0, 2 * ImGui::GetTextLineHeightWithSpacing())))
				{
					toBeSelected = key;
				}
				ImGui::NewLine();
			}
			ImGui::PopStyleVar();

			mSelectedCategory = toBeSelected;

			//ImGui::EndListBox();
		}
	}

	void SettingsWindow::DrawRightPanel()
	{
		if (mSubsystemsSettings.contains(mSelectedCategory))
		{
			if (!mSettings.contains(mSelectedCategory))
				mSettings.emplace(mSelectedCategory, Json());

			D_SERIALIZATION::Json& settingsJson = mSettings.at(mSelectedCategory);

			auto& drawFunc = mSubsystemsSettings[mSelectedCategory];
			auto changed = drawFunc(settingsJson);
		
			if (changed)
			{
				SaveSettings();
			}
		}


	}

	void SettingsWindow::SaveSettings() const
	{
		D_FILE::WriteJsonFile(D_ENGINE_CONTEXT::GetEngineSettingsPath(), mSettings);
	}

	void SettingsWindow::LoadSettings()
	{
		D_FILE::ReadJsonFile(D_ENGINE_CONTEXT::GetEngineSettingsPath(), mSettings);
	}
}
