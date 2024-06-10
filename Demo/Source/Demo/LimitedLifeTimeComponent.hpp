#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "LimitedLifeTimeComponent.generated.hpp"

namespace Demo
{
	class DClass(Serialize) LimitedLifeTimeComponent : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(LimitedLifeTimeComponent, D_ECS_COMP::BehaviourComponent, "Utils/Limited Lifetime", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;


	private:

		DField(Serialize)
		float							mTimeToLive;

		float							mStartTime;
	};
}

File_LimitedLifeTimeComponent_GENERATED
