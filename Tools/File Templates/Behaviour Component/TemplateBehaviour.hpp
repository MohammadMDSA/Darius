#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "%%CLASS_NAME%%.generated.hpp"

namespace %%NAMESPACE%%
{
	class DClass(Serialize) %%CLASS_NAME%% : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(%%CLASS_NAME%%, D_ECS_COMP::BehaviourComponent, "%%DISPLAY_NAME%%", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

	};
}

%%FILE_GENERATED%%
