#pragma once

#include "ComponentBase.hpp"

#include <Renderer/LightManager.hpp>

#ifndef D_ECS_COMP
#define D_ECS_COMP Darius::Scene::ECS::Components
#endif // !D_ECS_COMP

using namespace D_LIGHT;
using namespace D_SERIALIZATION;

namespace Darius::Scene::ECS::Components
{
	class LightComponent : public ComponentBase
	{
		D_H_COMP_BODY(LightComponent, ComponentBase, "Light");

	public:

#ifdef _DEBUG
		virtual bool					DrawDetails(float params[]) override;
#endif // _DEBUG

		// Serialization
		virtual void					Serialize(Json& j) const override;
		virtual void					Deserialize(Json const& j) override;

		// States
		virtual void					Start() override;
		virtual void					OnDestroy() override;

		virtual void					Update(float deltaTime) override;

		// Data intraction
		void							SetLightType(LightSourceType type);

		D_CH_R_FIELD(D_LIGHT::LightSourceType, LightType);
		D_CH_RW_FIELD(LightData, LightData);
		D_CH_R_FIELD(int, LightIndex);

	private:

		D_CORE::Signal<void()>			mChangeSignal;
	};
}