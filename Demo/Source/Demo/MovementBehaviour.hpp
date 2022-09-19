#pragma once

#include <Scene/EntityComponentSystem/Components/BehaviourComponent.hpp>
#include <Renderer/LightManager.hpp>

using namespace D_ECS_COMP;

namespace Demo
{
	class MovementBehaviour : public BehaviourComponent
	{
		D_H_COMP_BODY(MovementBehaviour, BehaviourComponent, "Movement", true);

	public:

#ifdef _DEBUG
		virtual bool					DrawDetails(float params[]) override;
#endif // _DEBUG

		// States
		virtual void					Start() override;

		virtual void					Update(float deltaTime) override;

	protected:

		D_CORE::Signal<void()>			mChangeSignal;
	};
}