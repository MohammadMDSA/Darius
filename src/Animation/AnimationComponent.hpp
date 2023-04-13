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

	protected:

		DField(Get[const, &, inline], Set[inline])
		AnimationState							mAnimState;

	private:

		void									CreateAnimationToJointIndexMap();

		DField(Get[inline], Set[inline], Serialize)
		bool									mRootMotion;

		DField(Resource, Serialize)
		D_CORE::Ref<AnimationResource>			mAnimation;

		D_CONTAINERS::DUnorderedMap<int, int>	mAnimationJointIndexMap; // Animation joint index to skeleton joint index

		D_CORE::Uuid							mMeshId;

		Darius_Animation_AnimationComponent_GENERATED
	};

}

File_AnimationComponent_GENERATED
