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

		struct AnimationState
		{
			enum eMode { kStopped, kPlaying, kLooping };
			eMode State;
			float Time;
			AnimationState() : State(kStopped), Time(0.0f) {}
		};

		D_H_COMP_BODY(AnimationComponent, D_ECS_COMP::ComponentBase, "Rendering/Animation", true, false);

	public:

#ifdef _DEBUG
		virtual bool					DrawDetails(float params[]) override;
#endif // _DEBUG

		// Serialization
		virtual void					Serialize(Json& j) const override {};
		virtual void					Deserialize(Json const& j) override {};

		virtual void					Update(float deltaTime) override;

		void							SetAnimation(ResourceHandle handle);

		D_CH_RW_FIELD_ACC(AnimationState, AnimState, protected);

	private:

		void							_SetAnimation(ResourceHandle handle);
		void							CreateAnimationToJointIndexMap();

		D_CORE::Ref<AnimationResource>			mAnimationResource;
		D_CONTAINERS::DUnorderedMap<int, int>	mAnimationJointIndexMap; // Animation joint index to skeleton joint index

		D_CORE::Uuid							mMeshId;

		D_CORE::Signal<void()>					mChangeSignal;
	};

}
