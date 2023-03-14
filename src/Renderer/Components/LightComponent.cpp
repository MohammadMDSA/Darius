#include "Renderer/pch.hpp"
#include "LightComponent.hpp"

#include <Math/Serialization.hpp>
#include <Scene/Utils/DetailsDrawer.hpp>

#include <imgui.h>

using namespace D_LIGHT;
using namespace D_SERIALIZATION;
using namespace DirectX;

namespace Darius::Graphics
{
	D_H_COMP_DEF(LightComponent);

	LightComponent::LightComponent() :
		D_ECS_COMP::ComponentBase(),
		mLightType(LightSourceType::PointLight),
		mLightIndex(-1),
		mConeInnerAngle(XMConvertToRadians(30)),
		mConeOuterAngle(XMConvertToRadians(45))
	{
		UpdateAngleData();
	}

	LightComponent::LightComponent(D_CORE::Uuid uuid) :
		D_ECS_COMP::ComponentBase(uuid),
		mLightType(LightSourceType::PointLight),
		mLightIndex(-1),
		mConeInnerAngle(XMConvertToRadians(30)),
		mConeOuterAngle(XMConvertToRadians(45))
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

				auto innerDeg = XMConvertToDegrees(mConeInnerAngle);
				auto outerDeg = XMConvertToDegrees(mConeOuterAngle);

				// Spot inner
				D_H_DETAILS_DRAW_PROPERTY("Inner Half Angle");
				if (ImGui::SliderAngle("##SpotInnerAngle", &mConeInnerAngle, 0, outerDeg))
				{
					changed = anglesChanged = true;
				}

				D_H_DETAILS_DRAW_PROPERTY("Outer Half Angle");
				if (ImGui::SliderAngle("##SpotOuterAngle", &mConeOuterAngle, innerDeg, 79.f))
					changed = anglesChanged = true;

				if (anglesChanged)
					UpdateAngleData();
			}

			if (mLightType != LightSourceType::DirectionalLight)
			{
				// Falloff Start
				D_H_DETAILS_DRAW_PROPERTY("Intencity");
				changed |= ImGui::DragFloat("##Intencity", &mLightData.Intencity, 0.01, 0, mLightData.Range, "%.3f");

				// Falloff End
				D_H_DETAILS_DRAW_PROPERTY("Range");
				changed |= ImGui::DragFloat("##Range", &mLightData.Range, 0.01, mLightData.Intencity, -1, "%.3f");
			}

			D_H_DETAILS_DRAW_PROPERTY("Casts Shadow");
			bool val = mLightData.CastsShadow;
			if (ImGui::Checkbox("##CastsShadow", &val))
			{
				mLightData.CastsShadow = val ? 1 : 0;
				changed = true;
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
			j["Range"] = mLightData.Range;
			j["Intencity"] = mLightData.Intencity;
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
			mLightData.Range = j["Range"];
			mLightData.Intencity = j["Intencity"];

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
