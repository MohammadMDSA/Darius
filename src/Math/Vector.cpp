#include "Vector.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <Core/Serialization/TypeSerializer.hpp>

#include <rttr/registration.h>

namespace Darius::Math
{
	const Vector3 Vector3::Up = Vector3(0.f, 1.f, 0.f);
	const Vector3 Vector3::Down = Vector3(0.f, -1.f, 0.f);
	const Vector3 Vector3::Left = Vector3(-1.f, 0.f, 0.f);
	const Vector3 Vector3::Right = Vector3(1.f, 0.f, 0.f);
	const Vector3 Vector3::Forward = Vector3(0.f, 0.f, -1.f);
	const Vector3 Vector3::Backward = Vector3(0.f, 0.f, 1.f);

	const Vector4 Vector4::Up = Vector4(0.f, 1.f, 0.f, 0.f);
	const Vector4 Vector4::Down = Vector4(0.f, -1.f, 0.f, 0.f);
	const Vector4 Vector4::Left = Vector4(-1.f, 0.f, 0.f, 0.f);
	const Vector4 Vector4::Right = Vector4(1.f, 0.f, 0.f, 0.f);
	const Vector4 Vector4::Forward = Vector4(0.f, 0.f, -1.f, 0.f);
	const Vector4 Vector4::Backward = Vector4(0.f, 0.f, 1.f, 0.f);

#ifdef _D_EDITOR

	bool DrawDetails(D_MATH::Vector3& elem, float params[])
	{
		auto valueChanged = false;
		auto values = reinterpret_cast<float*>(&elem);

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(&elem);

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		auto label = params[1] ? "R" : "X";
		if (ImGui::Button(label, buttonSize))
		{
			elem.SetX(params[0]);
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (params[1]) // Color
		{
			int val = (int)(values[0] * 255);
			if (ImGui::DragInt("##R", &val, 1, 0, 255))
			{
				values[0] = val / 255.f;
				valueChanged = true;
			}
		}
		else // Value
		{
			if (ImGui::DragFloat("##X", &values[0], 0.01f, 0.01f, 0.0f, "%.3f"))
				valueChanged = true;
		}

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 0 });
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		label = params[1] ? "G" : "Y";
		if (ImGui::Button(label, buttonSize))
		{
			elem.SetY(params[0]);
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		ImGui::SameLine();
		if (params[1]) // Color
		{
			int val = (int)(values[1] * 255);
			if (ImGui::DragInt("##G", &val, 1, 0, 255))
			{
				values[1] = val / 255.f;
				valueChanged = true;
			}
		}
		else // Value
		{
			if (ImGui::DragFloat("##Y", &values[1], 0.01f, 0.01f, 0.0f, "%.3f"))
				valueChanged = true;
		}

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 0 });
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		label = params[1] ? "B" : "Z";
		if (ImGui::Button(label, buttonSize))
		{
			elem.SetZ(params[0]);
			valueChanged = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		ImGui::SameLine();
		if (params[1]) // Color
		{
			int val = (int)(values[2] * 255);
			if (ImGui::DragInt("##B", &val, 1, 0, 255))
			{
				values[2] = val / 255.f;
				valueChanged = true;
			}
		}
		else // Value
		{
			if (ImGui::DragFloat("##Z", &values[2], 0.01f, 0.01f, 0.0f, "%.3f"))
				valueChanged = true;
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 0 });


		if (params[1])
		{
			ImGui::SameLine();

			valueChanged |= ImGui::ColorEdit3("MyColor##3", values, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha);
		}

		ImGui::PopStyleVar();

		ImGui::PopID();
		return valueChanged;
	}

	bool DrawDetails(D_MATH::Vector4& elem, float params[])
	{
		auto valueChanged = false;
		auto values = reinterpret_cast<float*>(&elem);

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(&elem);

		ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		auto label = params[1] ? "R" : "X";
		if (ImGui::Button(label, buttonSize))
		{
			elem.SetX(params[0]);
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		ImGui::SameLine();
		if (params[1]) // Color
		{
			int val = (int)(values[0] * 255);
			if (ImGui::DragInt("##R", &val, 1, 0, 255))
			{
				values[0] = val / 255.f;
				valueChanged = true;
			}
		}
		else // Value
		{
			if (ImGui::DragFloat("##X", &values[0], 0.01f, 0.01f, 0.0f, "%.3f"))
				valueChanged = true;
		}

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 0 });
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		label = params[1] ? "G" : "Y";
		if (ImGui::Button(label, buttonSize))
		{
			elem.SetY(params[0]);
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		ImGui::SameLine();
		if (params[1]) // Color
		{
			int val = (int)(values[1] * 255);
			if (ImGui::DragInt("##G", &val, 1, 0, 255))
			{
				values[1] = val / 255.f;
				valueChanged = true;
			}
		}
		else // Value
		{
			if (ImGui::DragFloat("##Y", &values[1], 0.01f, 0.01f, 0.0f, "%.3f"))
				valueChanged = true;
		}

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 0 });
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		label = params[1] ? "B" : "Z";
		if (ImGui::Button(label, buttonSize))
		{
			elem.SetZ(params[0]);
			valueChanged = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		ImGui::SameLine();
		if (params[1]) // Color
		{
			int val = (int)(values[2] * 255);
			if (ImGui::DragInt("##B", &val, 1, 0, 255))
			{
				values[2] = val / 255.f;
				valueChanged = true;
			}
		}
		else // Value
		{
			if (ImGui::DragFloat("##Z", &values[2], 0.01f, 0.01f, 0.0f, "%.3f"))
				valueChanged = true;
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 0 });
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.8f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.9f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.8f, 0.1f, 1.0f });
		ImGui::PushFont(boldFont);
		label = params[1] ? "A" : "W";
		if (ImGui::Button(label, buttonSize))
		{
			elem.SetW(params[0]);
			valueChanged = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		ImGui::SameLine();
		if (params[1]) // Color
		{
			if (ImGui::DragFloat("##A", &values[3], 0.01f, 0.f, 1.f))
			{
				valueChanged = true;
			}
		}
		else // Value
		{
			if (ImGui::DragFloat("##W", &values[3], 0.01f, 0.01f, 0.0f, "%.3f"))
				valueChanged = true;
		}
		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 0 });

		if (params[1])
		{
			ImGui::SameLine();
			valueChanged |= ImGui::ColorEdit4("MyColor##4", values, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		}

		ImGui::PopStyleVar();

		ImGui::PopID();
		return valueChanged;
	}

#endif // _D_EDITOR

	INVOKE_STATIC_CONSTRUCTOR(Vector3);

	void Vector3::StaticConstructor()
	{
		rttr::registration::class_<Vector3>("Darius::Math::Vector3")
			.property("X", &Vector3::x)
			.property("Y", &Vector3::y)
			.property("Z", &Vector3::z);
	}

	void Vector3::StaticDestructor()
	{

	}

	void Vector4::StaticConstructor()
	{
		rttr::registration::class_<Vector4>("Darius::Math::Vector4")
			.property("X", &Vector4::x)
			.property("Y", &Vector4::y)
			.property("Z", &Vector4::z)
			.property("W", &Vector4::w);
	}

	void Vector4::StaticDestructor()
	{

	}

}