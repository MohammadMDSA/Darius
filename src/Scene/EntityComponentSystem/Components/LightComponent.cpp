#include "Scene/pch.hpp"
#include "LightComponent.hpp"

#include "Scene/Utils/Serializer.hpp"

using namespace D_LIGHT;

namespace Darius::Scene::ECS::Components
{
	D_H_COMP_DEF(LightComponent);

	LightComponent::LightComponent() :
		ComponentBase(),
		mLightType(LightSourceType::PointLight),
		mLightIndex(-1)
	{
	}

	LightComponent::LightComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mLightType(LightSourceType::PointLight),
		mLightIndex(-1)
	{
	}

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
		D_LIGHT::UpdateLight(mLightType, mLightIndex, &GetTransformC(), IsActive(), mLightData);
	}

#ifdef _DEBUG
	bool LightComponent::DrawDetails(float params[])
	{


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
