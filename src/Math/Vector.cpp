#include "Vector.hpp"

#ifdef _D_EDITOR
#include "imgui.h"
#include "imgui_internal.h"
#endif

#include <rttr/registration.h>

#include "Scalar.sgenerated.hpp"
#include "Vector.sgenerated.hpp"

#define VectorElementEdit(VecType, Comp) \
{ \
	VecType::ElementType value = elem.Get##Comp(); \
	if(ImGui::DragFloat("##"#Comp, &value, 0.01f, 0.01f, 0.0f, "%.3f")) \
	{ \
		elem.Set##Comp(value); \
		valueChanged = true; \
	} \
}

namespace Darius::Math
{
	const Vector2 Vector2::Zero = Vector2(0.f);
	const Vector2 Vector2::One = Vector2(1.f);
	const Vector2 Vector2::UnitX = Vector2(1.f, 0.f);
	const Vector2 Vector2::UnitY = Vector2(0.f, 1.f);

	const Vector3 Vector3::Up = Vector3(0.f, 1.f, 0.f);
	const Vector3 Vector3::Down = Vector3(0.f, -1.f, 0.f);
	const Vector3 Vector3::Left = Vector3(-1.f, 0.f, 0.f);
	const Vector3 Vector3::Right = Vector3(1.f, 0.f, 0.f);
	const Vector3 Vector3::Forward = Vector3(0.f, 0.f, -1.f);
	const Vector3 Vector3::Backward = Vector3(0.f, 0.f, 1.f);
	const Vector3 Vector3::Zero = Vector3(0.f, 0.f, 0.f);
	const Vector3 Vector3::One = Vector3(1.f, 1.f, 1.f);
	const Vector3 Vector3::UnitX = Vector3(1.f, 0.f, 0.f);
	const Vector3 Vector3::UnitY = Vector3(0.f, 1.f, 0.f);
	const Vector3 Vector3::UnitZ = Vector3(0.f, 0.f, 1.f);

	const Vector4 Vector4::Up = Vector4(0.f, 1.f, 0.f, 0.f);
	const Vector4 Vector4::Down = Vector4(0.f, -1.f, 0.f, 0.f);
	const Vector4 Vector4::Left = Vector4(-1.f, 0.f, 0.f, 0.f);
	const Vector4 Vector4::Right = Vector4(1.f, 0.f, 0.f, 0.f);
	const Vector4 Vector4::Forward = Vector4(0.f, 0.f, -1.f, 0.f);
	const Vector4 Vector4::Backward = Vector4(0.f, 0.f, 1.f, 0.f);
	const Vector4 Vector4::Zero = Vector4(0.f, 0.f, 0.f, 0.f);
	const Vector4 Vector4::One = Vector4(1.f, 1.f, 1.f, 1.f);
	const Vector4 Vector4::UnitX = Vector4(1.f, 0.f, 0.f, 0.f);
	const Vector4 Vector4::UnitY = Vector4(0.f, 1.f, 0.f, 0.f);
	const Vector4 Vector4::UnitZ = Vector4(0.f, 0.f, 1.f, 0.f);
	const Vector4 Vector4::UnitW = Vector4(0.f, 0.f, 0.f, 1.f);

#ifdef _D_EDITOR

	bool DrawDetails(D_MATH::Vector2& elem, Vector2 const& defaultValue)
	{
		auto valueChanged = false;


		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(&elem);
		ImGui::BeginGroup();

		ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.9f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushFont(boldFont);
		if(ImGui::Button("X", buttonSize))
		{
			elem.SetX(defaultValue.GetX());
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		VectorElementEdit(Vector2, X);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {5, 0});
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.3f, 0.8f, 0.3f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushFont(boldFont);
		if(ImGui::Button("Y", buttonSize))
		{
			elem.SetY(defaultValue.GetY());
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		ImGui::SameLine();

		VectorElementEdit(Vector2, Y);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		ImGui::EndGroup();
		ImGui::PopID();
		return valueChanged;
	}

	bool DrawDetails(D_MATH::Vector3& elem, D_MATH::Vector3 const& defaultValue)
	{
		auto valueChanged = false;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(&elem);
		ImGui::BeginGroup();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.9f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushFont(boldFont);
		if(ImGui::Button("X", buttonSize))
		{
			elem.SetX(defaultValue.GetX());
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();

		VectorElementEdit(Vector3, X);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {5, 0});
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.3f, 0.8f, 0.3f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushFont(boldFont);
		if(ImGui::Button("Y", buttonSize))
		{
			elem.SetY(defaultValue.GetY());
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		ImGui::SameLine();

		VectorElementEdit(Vector3, Y);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {5, 0});
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.2f, 0.35f, 0.9f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushFont(boldFont);
		if(ImGui::Button("Z", buttonSize))
		{
			elem.SetZ(defaultValue.GetZ());
			valueChanged = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		ImGui::SameLine();

		VectorElementEdit(Vector3, Z);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();

		ImGui::EndGroup();
		ImGui::PopID();
		return valueChanged;
	}

	bool DrawDetails(D_MATH::Vector4& elem, Vector4 const& defaultValue)
	{
		auto valueChanged = false;
		auto values = reinterpret_cast<float*>(&elem);

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(&elem);

		ImGui::PushMultiItemsWidths(4, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.9f, 0.2f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.8f, 0.1f, 0.15f, 1.0f});
		ImGui::PushFont(boldFont);
		if(ImGui::Button("X", buttonSize))
		{
			elem.SetX(defaultValue.GetX());
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		ImGui::SameLine();

		VectorElementEdit(Vector4, X);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {5, 0});
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.3f, 0.8f, 0.3f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.2f, 0.7f, 0.2f, 1.0f});
		ImGui::PushFont(boldFont);
		if(ImGui::Button("Y", buttonSize))
		{
			elem.SetY(defaultValue.GetY());
			valueChanged = true;
		}
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		ImGui::SameLine();

		VectorElementEdit(Vector4, Y);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {5, 0});
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.2f, 0.35f, 0.9f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.1f, 0.25f, 0.8f, 1.0f});
		ImGui::PushFont(boldFont);
		if(ImGui::Button("Z", buttonSize))
		{
			elem.SetZ(defaultValue.GetZ());
			valueChanged = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		ImGui::SameLine();

		VectorElementEdit(Vector4, Z);

		ImGui::PopItemWidth();
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {5, 0});
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4 {0.8f, 0.8f, 0.15f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4 {0.9f, 0.9f, 0.2f, 1.0f});
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4 {0.8f, 0.8f, 0.1f, 1.0f});
		ImGui::PushFont(boldFont);
		if(ImGui::Button("W", buttonSize))
		{
			elem.SetW(defaultValue.GetW());
			valueChanged = true;
		}

		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2 {0, 0});

		ImGui::SameLine();

		VectorElementEdit(Vector4, W);

		ImGui::PopItemWidth();
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

	rttr::registration::class_<D_MATH::Vector2>("Darius::Math::Vector2")
		.property("X", &D_MATH::Vector2::GetX, &D_MATH::Vector2::SetX) (rttr::metadata("NO_SERIALIZE", true))
		.property("Y", &D_MATH::Vector2::GetY, &D_MATH::Vector2::SetY) (rttr::metadata("NO_SERIALIZE", true))
		.property("_data", &D_MATH::Vector2::GetData, &D_MATH::Vector2::SetData);

	rttr::registration::class_<D_MATH::Vector3>("Darius::Math::Vector3")
		.property("X", &D_MATH::Vector3::GetX, &D_MATH::Vector3::SetX) (rttr::metadata("NO_SERIALIZE", true))
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