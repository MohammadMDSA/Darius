#include "Vector.hpp"
#include "imgui.h"
#include "imgui_internal.h"

#include <Core/Serialization/TypeSerializer.hpp>

#include <rttr/registration.h>

#include "Scalar.sgenerated.hpp"
#include "Vector.sgenerated.hpp"

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


}

RTTR_REGISTRATION
{
	rttr::registration::class_<D_MATH::Scalar>("Darius::Math::Scalar")
		.property("Value", &D_MATH::Scalar::x);

	rttr::registration::class_<D_MATH::Vector3>("Darius::Math::Vector3")
		.property("X", &D_MATH::Vector3::GetX, &D_MATH::Vector3::SetX) ( rttr::metadata("NO_SERIALIZE", true))
		.property("Y", &D_MATH::Vector3::GetY, &D_MATH::Vector3::SetY) (rttr::metadata("NO_SERIALIZE", true))
		.property("Z", &D_MATH::Vector3::GetZ, &D_MATH::Vector3::SetZ) (rttr::metadata("NO_SERIALIZE", true))
		.property("_data", &D_MATH::Vector3::GetData, &D_MATH::Vector3::SetData);

	rttr::registration::class_<DirectX::XMFLOAT3>("DirectX::XMFLOAT3")
		.property("X", &DirectX::XMFLOAT3::x)
		.property("Y", &DirectX::XMFLOAT3::y)
		.property("Z", &DirectX::XMFLOAT3::z);

	rttr::registration::class_<D_MATH::Vector4>("Darius::Math::Vector4")
		.property("X", &D_MATH::Vector4::GetX, &D_MATH::Vector4::SetX) (rttr::metadata("NO_SERIALIZE", true))
		.property("Y", &D_MATH::Vector4::GetY, &D_MATH::Vector4::SetY) (rttr::metadata("NO_SERIALIZE", true))
		.property("Z", &D_MATH::Vector4::GetZ, &D_MATH::Vector4::SetZ) (rttr::metadata("NO_SERIALIZE", true))
		.property("W", &D_MATH::Vector4::GetW, &D_MATH::Vector4::SetW) (rttr::metadata("NO_SERIALIZE", true))
		.property("_data", &D_MATH::Vector4::GetData, &D_MATH::Vector4::SetData);

	rttr::registration::class_<DirectX::XMFLOAT4>("DirectX::XMFLOAT4")
		.property("X", &DirectX::XMFLOAT4::x)
		.property("Y", &DirectX::XMFLOAT4::y)
		.property("Z", &DirectX::XMFLOAT4::z)
		.property("W", &DirectX::XMFLOAT4::w);

}