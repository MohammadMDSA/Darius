#pragma once

#include <Renderer/Light/LightManager.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "LightComponent.generated.hpp"

#ifndef D_GRAPHICS
#define D_GRAPHICS Darius::Graphics
#endif

namespace Darius::Graphics
{
	class DClass(Serialize) LightComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(LightComponent, D_ECS_COMP::ComponentBase, "Rendering/Light", true);

	public:
		Darius_Graphics_LightComponent_GENERATED

#ifdef _DEBUG
		virtual bool					DrawDetails(float params[]) override;
#endif // _DEBUG

		// Serialization
		virtual void					Serialize(D_SERIALIZATION::Json& j) const override;
		virtual void					Deserialize(D_SERIALIZATION::Json const& j) override;

		// States
		virtual INLINE void				Start() override { SetLightType(mLightType); }
		virtual INLINE void				OnDestroy() override { D_LIGHT::ReleaseLight(mLightType, mLightIndex); }

		// Gameobject events
		virtual void					OnActivate() override;
		virtual void					OnDeactivate() override;

		virtual void					Update(float deltaTime) override;

		// Data intraction
		void							SetLightType(D_LIGHT::LightSourceType type);

		D_CH_RW_FIELD(D_LIGHT::LightData, LightData);
		D_CH_RW_FIELD(float, ConeOuterAngle);
		D_CH_RW_FIELD(float, ConeInnerAngle);

	private:

		DField(Get[inline])
		int								mLightIndex;

		DField(Get[inline, const, &])
		D_LIGHT::LightSourceType		mLightType;

	protected:
		INLINE void						UpdateAngleData()
		{
			auto cosOuter = D_MATH::Cos(mConeOuterAngle);
			mLightData.SpotAngles.x = 1.f / (D_MATH::Cos(mConeInnerAngle) - cosOuter);
			mLightData.SpotAngles.y = cosOuter;
		}
	};
}

File_LightComponent_GENERATED
