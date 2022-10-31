#pragma once

#include "AnimationResource.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

namespace Darius::Animation
{

	class AnimationComponent : public D_ECS_COMP::ComponentBase
	{
		D_H_COMP_BODY(AnimationComponent, D_ECS_COMP::ComponentBase, "Rendering/Animation", true, false);

	public:

#ifdef _DEBUG
		virtual bool					DrawDetails(float params[]) override;
#endif // _DEBUG

		// Serialization
		virtual void					Serialize(Json& j) const override {};
		virtual void					Deserialize(Json const& j) override {};

		virtual void					Update(float deltaTime) override {};

		void							SetAnimation(ResourceHandle handle);
	private:

		void							_SetAnimation(ResourceHandle handle);

		D_CORE::Ref<AnimationResource>	mAnimationResource;

		D_CORE::Signal<void()>			mChangeSignal;
	};

}
