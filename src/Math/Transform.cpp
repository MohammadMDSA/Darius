#include "Transform.hpp"

#include <imgui.h>

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
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthStretch, 1);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch, 2);


			// Translation
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Location");

			ImGui::TableSetColumnIndex(1);
			if (DrawDetails(elem.Translation, Vector3::Zero))
			{
				valueChanged = true;
			}

			// Rotation
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Rotation");

			ImGui::TableSetColumnIndex(1);
			if (DrawDetails(elem.Rotation, Quaternion::Identity))
			{
				valueChanged = true;
			}

			// Scale
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Scale");

			ImGui::TableSetColumnIndex(1);
			if (DrawDetails(elem.Scale, Vector3::One))
			{
				valueChanged = true;
			}

			ImGui::EndTable();
		}

		return valueChanged;
	}

#endif // _D_EDITOR

}