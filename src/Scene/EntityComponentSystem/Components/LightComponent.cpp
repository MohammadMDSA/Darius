#include "Scene/pch.hpp"
#include "LightComponent.hpp"

#include "Scene/Utils/Serializer.hpp"
#include "Scene/Utils/DetailsDrawer.hpp"

#include <imgui.h>

using namespace D_LIGHT;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(LightComponent);

	LightComponent::LightComponent() :
		ComponentBase(),
		mLightType(LightSourceType::PointLight),
		mLightIndex(-1),
		mConeInnerAngle(D_MATH::XMConvertToRadians(30)),
		mConeOuterAngle(D_MATH::XMConvertToRadians(45))
	{
		UpdateAngleData();
	}

	LightComponent::LightComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mLightType(LightSourceType::PointLight),
		mLightIndex(-1),
		mConeInnerAngle(D_MATH::XMConvertToRadians(30)),
		mConeOuterAngle(D_MATH::XMConvertToRadians(45))
	{
		UpdateAngleData();
	}

	void LightComponent::Update(float deltaTime)
	{
		if (mLightIndex >= 0)
			D_LIGHT::UpdateLight(mLightType, mLightIndex, GetTransform(), IsActive(), mLightData);
	}

#ifdef _DEBUG
	bool LightComponent::DrawDetails(float params[])
	{
		bool changed = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE()

			// Light type
			D_H_DETAILS_DRAW_PROPERTY("Light Type");
		int lightType = (int)mLightType;
		if (ImGui::Combo("##LightTyp", &lightType, "DirectionalLight\0PointLight\0SpotLight\0\0"))
		{
			SetLightType((LightSourceType)lightType);
			changed = true;
		}

		if (mLightIndex >= 0)
		{
			// Light type
			D_H_DETAILS_DRAW_PROPERTY("Color");
			float defC[] = D_H_DRAW_DETAILS_MAKE_VEC_PARAM_COLOR;
			changed |= D_MATH::DrawDetails(*(D_MATH::Vector3*)&mLightData.Color, defC);

			if (mLightType == LightSourceType::SpotLight)
			{
				bool anglesChanged = false;

				auto innerDeg = D_MATH::XMConvertToDegrees(mConeInnerAngle);
				auto outerDeg = D_MATH::XMConvertToDegrees(mConeOuterAngle);

				// Spot inner
				D_H_DETAILS_DRAW_PROPERTY("Inner Half Angle");
				if (ImGui::SliderAngle("##SpotInnerAngle", &mConeInnerAngle, 0, outerDeg))
				{
					changed = anglesChanged = true;
				}

				D_H_DETAILS_DRAW_PROPERTY("Outer Half Angle");
				if (ImGui::SliderAngle("##SpotOuterAngle", &mConeOuterAngle, innerDeg, 179))
					changed = anglesChanged = true;

				if (anglesChanged)
					UpdateAngleData();
			}

			if (mLightType != LightSourceType::DirectionalLight)
			{
				// Falloff Start
				D_H_DETAILS_DRAW_PROPERTY("Falloff Start");
				changed |= ImGui::DragFloat("##FalloffStart", &mLightData.FalloffStart, 0.01, 0, mLightData.FalloffEnd, "%.3f");

				// Falloff End
				D_H_DETAILS_DRAW_PROPERTY("Falloff End");
				changed |= ImGui::DragFloat("##FalloffEnd", &mLightData.FalloffEnd, 0.01, mLightData.FalloffStart, -1, "%.3f");
			}
		}
		else
		{
			ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Light with requested type was not accuired successfully. Change Type or request again.");

			if (ImGui::Button("Retry"))
				SetLightType(mLightType);
		}

		D_H_DETAILS_DRAW_END_TABLE();

		return changed;
	}
#endif

	void LightComponent::Serialize(Json& j) const
	{
		j["Type"] = mLightType;
		if (mLightIndex >= 0)
		{

			j["Color"] = D_MATH::Vector3(D_MATH::Vector4(mLightData.Color));
			j["FalloffEnd"] = mLightData.FalloffEnd;
			j["FalloffStart"] = mLightData.FalloffStart;
			j["SpotInnerAngle"] = mConeInnerAngle;
			j["SpotOuterAngle"] = mConeOuterAngle;
		}
	}

	void LightComponent::Deserialize(Json const& j)
	{
		SetLightType(j["Type"].get<LightSourceType>());
		if (mLightIndex >= 0 && j.contains("Color"))
		{

			mLightData.Color = (XMFLOAT3)j["Color"].get<D_MATH::Vector3>();
			mLightData.FalloffEnd = j["FalloffEnd"];
			mLightData.FalloffStart = j["FalloffStart"];

			if (j.contains("SpotInnerAngle"))
				mConeInnerAngle = j["SpotInnerAngle"];
			if (j.contains("SpotOuterAngle"))
				mConeOuterAngle = j["SpotOuterAngle"];
		}
		UpdateAngleData();
	}

	void LightComponent::SetLightType(LightSourceType type)
	{
		if (mLightIndex >= 0)
			mLightIndex = D_LIGHT::SwapLightSource(type, mLightType, mLightIndex);
		else
			mLightIndex = D_LIGHT::AccuireLightSource(type);

		mLightType = type;
	}

	void LightComponent::OnActivate()
	{
		if (mLightIndex >= 0)
			D_LIGHT::UpdateLight(mLightType, mLightIndex, GetTransform(), IsActive(), mLightData);
	}

	void LightComponent::OnDeactivate()
	{
		if (mLightIndex >= 0)
			D_LIGHT::UpdateLight(mLightType, mLightIndex, GetTransform(), IsActive(), mLightData);
	}

}
