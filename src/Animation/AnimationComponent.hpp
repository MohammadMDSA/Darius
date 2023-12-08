#pragma once

#include "AnimationResource.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include "AnimationComponent.generated.hpp"

#ifndef D_ANIMATION
#define D_ANIMATION Darius::Animation
#endif // !D_ANIMATION

namespace Darius::Animation
{

	class DClass(Serialize) AnimationComponent : public D_ECS_COMP::ComponentBase
	{
		GENERATED_BODY();

	public:


		struct AnimationState
		{
			enum eMode { kStopped, kPlaying, kLooping };
			eMode State;
			float Time;
			AnimationState() : State(kStopped), Time(0.0f) {}
		};

		D_H_COMP_BODY(AnimationComponent, D_ECS_COMP::ComponentBase, "Rendering/Animation", true);

	public:

#ifdef _D_EDITOR
		virtual bool							DrawDetails(float params[]) override;
#endif // _D_EDITOR

		virtual void							Update(float deltaTime) override;
		virtual void							Awake() override;

		INLINE AnimationResource*				GetAnimation() const { return mAnimation.Get(); }
		void									SetAnimation(AnimationResource* animation);
		void									SetAnimState(AnimationState const& value) { mAnimState = value; }
		INLINE void								SetRootMotion(bool rootMotion) { mRootMotion = rootMotion; }

		INLINE AnimationState const&			GetAnimState() const { return mAnimState; }
		INLINE bool								IsRootMotion() const { return mRootMotion; }

	protected:

		DField()
		AnimationState							mAnimState;

	private:

		void									CreateAnimationToJointIndexMap();

		DField(Serialize)
		bool									mRootMotion;

		DField(Serialize)
		D_RESOURCE::ResourceRef<AnimationResource> mAnimation;

		D_CONTAINERS::DUnorderedMap<int, int>	mAnimationJointIndexMap; // Animation joint index to skeleton joint index

		D_CORE::Uuid							mMeshId;
	};

}

File_AnimationComponent_GENERATED
