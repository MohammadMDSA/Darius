#pragma once

#include <Math/VectorMath.hpp>
#include <Renderer/FrameResource.hpp>
#include <Utils/Common.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

#ifdef _D_EDITOR

#ifndef D_SCENE_INS_DRAW
#define D_SCENE_DET_DRAW Darius::Scene::Utils::DetailsDrawer
#endif // !D_SCENE_INS_DRAW

using namespace D_MATH;
using namespace D_RENDERER_FRAME_RESOUCE;

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

			valueChanged |= ImGui::ColorEdit3("MyColor##3", (float*)&values, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
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
			valueChanged |= ImGui::ColorEdit4("MyColor##3", (float*)&values, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		}

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

		float def[] = { 0.f, 0.f };
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

	bool DrawDetails(D_RENDERER_FRAME_RESOUCE::Material& mat, float params[])
	{
		bool valueChanged = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];


		if (ImGui::BeginTable("mat editor", 2, ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);


			// Translation
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Diffuse Color");

			ImGui::TableSetColumnIndex(1);
			float defL[] = { 0.f, 1.f };
			if (DrawDetails(mat.DifuseAlbedo, defL))
			{
				valueChanged = true;
			}

			// Rotation
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Fresnel");

			ImGui::TableSetColumnIndex(1);
			float defR[] = { 0.f, 1.f };
			if (DrawDetails(mat.FresnelR0, defR))
			{
				valueChanged = true;
			}

			// Scale
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Scale");

			ImGui::TableSetColumnIndex(1);
			float defS[] = { 1.f, 0.f };
			if (ImGui::DragFloat("##X", &mat.Roughness, 0.01f, 0.f, 1.f, "% .3f"))
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