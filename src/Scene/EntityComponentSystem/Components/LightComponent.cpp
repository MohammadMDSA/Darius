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
		mLightIndex(-1)
	{ }

	LightComponent::LightComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mLightType(LightSourceType::PointLight),
		mLightIndex(-1)
	{ }

	void LightComponent::OnDestroy()
	{
		D_LIGHT::ReleaseLight(mLightType, mLightIndex);
	}

	void LightComponent::Start()
	{
		SetLightType(mLightType);
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

		if (ImGui::BeginTable("##componentLayout", 2, ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);


			// Light type
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Light Type");

			ImGui::TableSetColumnIndex(1);
			int lightType = (int)mLightType;
			if (ImGui::Combo("##LightTyp", &lightType, "DirectionalLight\0PointLight\0SpotLight\0\0"))
			{
				SetLightType((LightSourceType)lightType);
				changed = true;
			}

			if (mLightIndex >= 0)
			{
				// Light type
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Color");

				float defC[] = { 0.f, 1.f };
				ImGui::TableSetColumnIndex(1);
				changed |= D_SCENE_DET_DRAW::DrawDetails(*(D_MATH::Vector3*)&mLightData.Color, defC);

				if (mLightType == LightSourceType::SpotLight)
				{
					// Spot Power
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Spot Power");

					ImGui::TableSetColumnIndex(1);
					float powerLog = D_MATH::Log(mLightData.SpotPower);
					if (ImGui::DragFloat("##SpotPower", &powerLog, 0.01, 0.001, -1, "%.3f"))
					{
						changed = true;
						mLightData.SpotPower = D_MATH::Exp(powerLog);
					}
				}

				if (mLightType != LightSourceType::DirectionalLight)
				{
					// Falloff Start
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Falloff Start");

					ImGui::TableSetColumnIndex(1);
					changed |= ImGui::DragFloat("##FalloffStart", &mLightData.FalloffStart, 0.01, 0, mLightData.FalloffStart, "%.3f");

					// Falloff End
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::Text("Falloff End");

					ImGui::TableSetColumnIndex(1);
					changed |= ImGui::DragFloat("##FalloffEnd", &mLightData.FalloffEnd, 0.01, mLightData.FalloffStart, -1, "%.3f");
				}
			}
			else
			{
				ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "Light with requested type was not accuired successfully. Change Type or request again.");

				if (ImGui::Button("Retry"))
					SetLightType(mLightType);
			}

			ImGui::EndTable();
		}

		return false;
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
			j["SpotPower"] = mLightData.SpotPower;
		}
	}

	void LightComponent::Deserialize(Json const& j)
	{
		SetLightType(j["Type"].get<LightSourceType>());
		if (mLightIndex >= 0 && j.contains("Color"))
		{
			mLightData.Color = D_MATH::Vector4(j["Color"].get<D_MATH::Vector3>(), 1.f);
			mLightData.FalloffEnd = j["FalloffEnd"];
			mLightData.FalloffStart = j["FalloffStart"];
			mLightData.SpotPower = j["SpotPower"];
		}
	}

	void LightComponent::SetLightType(LightSourceType type)
	{
		if (mLightIndex >= 0)
			mLightIndex = D_LIGHT::SwapLightSource(type, mLightType, mLightIndex);
		else
			mLightIndex = D_LIGHT::AccuireLightSource(type);

		mLightType = type;
	}

}
