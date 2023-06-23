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

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
		virtual void					OnGizmo() const override;
#endif // _DEBUG

		// States
		virtual void					Awake() override;

		INLINE void						SetColor(D_MATH::Color const& value)
		{
			mLightData.Color = (DirectX::XMFLOAT3)D_MATH::Vector3(value);
		}

		INLINE D_MATH::Color const&		GetColor() const
		{
			return D_MATH::Color(D_MATH::Vector3(mLightData.Color));
		}

		INLINE void						SetRenge(float const& value) { mLightData.Range = value; }
		INLINE float const&				GetRange() const { return mLightData.Range; }

		INLINE void						SetConeOuterAngle(float const& val) { mConeOuterAngle = val; UpdateAngleData(); }
		INLINE void						SetConeInnerAngle(float const& val) { mConeInnerAngle = val; UpdateAngleData(); }


	private:

		DField(Get[inline], Set[inline], Serialize)
		D_LIGHT::LightSourceType		mLightType;

		DField(Get[inline, const, &], Set[inline], Serialize)
		D_LIGHT::LightData				mLightData;

		DField(Get[inline], Serialize)
		float							mConeOuterAngle;

		DField(Get[inline], Serialize)
		float							mConeInnerAngle;

	protected:
		INLINE void						UpdateAngleData()
		{
			auto cosOuter = D_MATH::Cos(mConeOuterAngle);
			mLightData.SpotAngles.x = 1.f / (D_MATH::Cos(mConeInnerAngle) - cosOuter);
			mLightData.SpotAngles.y = cosOuter;
		}

	public:
		Darius_Graphics_LightComponent_GENERATED
	};
}

File_LightComponent_GENERATED
