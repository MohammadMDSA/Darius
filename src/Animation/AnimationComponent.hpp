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

#ifdef _D_EDITOR
		virtual bool					DrawDetails(float params[]) override;
#endif // _D_EDITOR

		// Serialization
		virtual void					Serialize(D_SERIALIZATION::Json& j) const override;
		virtual void					Deserialize(D_SERIALIZATION::Json const& j) override;

		virtual void					Update(float deltaTime) override;
		virtual void					Awake() override;

		void							SetAnimation(D_RESOURCE::ResourceHandle handle);

		D_CH_RW_FIELD(bool, RootMotion);
		D_CH_RW_FIELD_ACC(AnimationState, AnimState, protected);

	private:

		void							_SetAnimation(D_RESOURCE::ResourceHandle handle);
		void							CreateAnimationToJointIndexMap();

		D_CORE::Ref<AnimationResource>			mAnimationResource;
		D_CONTAINERS::DUnorderedMap<int, int>	mAnimationJointIndexMap; // Animation joint index to skeleton joint index

		D_CORE::Uuid							mMeshId;

	};

}
