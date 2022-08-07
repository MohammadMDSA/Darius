#pragma once

#include <Math/VectorMath.hpp>
#include <Utils/Common.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

#ifdef _D_EDITOR

#ifndef D_SCENE_INS_DRAW
#define D_SCENE_DET_DRAW Darius::Scene::Utils::DetailsDrawer
#endif // !D_SCENE_INS_DRAW

using namespace D_MATH;

namespace Darius::Scene::Utils::DetailsDrawer
{
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
		if (ImGui::Button("X", buttonSize))
		{
			elem.SetX(params[0]);
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##X", &values[0], 0.01f, 0.01f, 0.0f, "%.3f"))
			valueChanged = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
		{
			elem.SetY(params[0]);
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Y", &values[1], 0.01f, 0.01f, 0.0f, "%.3f"))
			valueChanged = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
		{
			elem.SetZ(params[0]);
			valueChanged = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		if (ImGui::DragFloat("##Z", &values[2], 0.01f, 0.01f, 0.0f, "%.3f"))
			valueChanged = true;
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::PopID();
		return valueChanged;
	}

	bool DrawDetails(Quaternion& quat, float params[])
	{
		Vector3 radian = quat.Angles();
		Vector3 deg;

		deg.SetX(XMConvertToDegrees(radian.GetX()));
		deg.SetY(XMConvertToDegrees(radian.GetY()));
		deg.SetZ(XMConvertToDegrees(radian.GetZ()));

		float def[] = { 0.f };
		if (DrawDetails(deg, def))
		{
			quat = Quaternion(XMConvertToRadians(deg.GetX()), XMConvertToRadians(deg.GetY()), XMConvertToRadians(deg.GetZ()));
			return true;
		}
		return false;
	}

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
			float defL[] = { 0.f };
			if (DrawDetails(elem.Translation, defL))
			{
				valueChanged = true;
			}

			// Rotation
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Rotation");

			ImGui::TableSetColumnIndex(1);
			float defR[] = { 0.f };
			if (DrawDetails(elem.Rotation, defR))
			{
				valueChanged = true;
			}

			// Scale
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Scale");

			ImGui::TableSetColumnIndex(1);
			float defS[] = { 1.f };
			if (DrawDetails(elem.Scale, defS))
			{
				valueChanged = true;
			}



			ImGui::EndTable();
		}

		return valueChanged;
	}


	template<typename T>
	bool DrawDetails(T& elem, float params[])
	{
		return elem.DrawDetails();
	}

}

#endif