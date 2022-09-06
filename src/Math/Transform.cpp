#include "Transform.hpp"

#include <imgui/imgui.h>

namespace Darius::Math
{
#ifdef _D_EDITOR

	bool DrawDetails(Transform& elem, float params[])
	{
		bool valueChanged = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];


		if (ImGui::BeginTable("vec3 editor", 2, ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);


			// Translation
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Location");

			ImGui::TableSetColumnIndex(1);
			float defL[] = { 0.f, 0.f };
			if (DrawDetails(elem.Translation, defL))
			{
				valueChanged = true;
			}

			// Rotation
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Rotation");

			ImGui::TableSetColumnIndex(1);
			float defR[] = { 0.f, 0.f };
			if (DrawDetails(elem.Rotation, defR))
			{
				valueChanged = true;
			}

			// Scale
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Scale");

			ImGui::TableSetColumnIndex(1);
			float defS[] = { 1.f, 0.f };
			if (DrawDetails(elem.Scale, defS))
			{
				valueChanged = true;
			}

			ImGui::EndTable();
		}

		return valueChanged;
	}

#endif // _D_EDITOR

}