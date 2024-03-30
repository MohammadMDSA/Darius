#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>

#include "ComponentReferencer.generated.hpp"

namespace Demo
{
	class DClass(Serialize) ComponentReferencer : public D_ECS_COMP::BehaviourComponent
	{
		GENERATED_BODY();

		D_H_BEHAVIOUR_COMP_BODY(ComponentReferencer, D_ECS_COMP::BehaviourComponent, "Test/Component Referencer", true, true);

	public:

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

		void							SetOtherTrans(D_MATH::TransformComponent* trans);

	private:

		DField(Serialize)
		D_ECS::CompRef<D_MATH::TransformComponent>	mOtherTrans;

	};
}

File_ComponentReferencer_GENERATED
