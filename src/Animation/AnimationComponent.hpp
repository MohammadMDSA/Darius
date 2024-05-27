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

			INLINE bool operator== (AnimationState const& other)
			{
				return State == other.State && Time == other.Time;
				D_STATIC_ASSERT(sizeof(AnimationState) == 8); // To ensure fixing this function if properties changed
			}
		};

		D_H_COMP_BODY(AnimationComponent, D_ECS_COMP::ComponentBase, "Rendering/Animation", true);

	public:

#ifdef _D_EDITOR
		virtual bool							DrawDetails(float params[]) override;
#endif // _D_EDITOR

		virtual void							Update(float deltaTime) override;
		virtual void							Awake() override;
		virtual void							Start() override;

		INLINE AnimationResource*				GetAnimation() const { return mAnimation.Get(); }
		void									SetAnimation(AnimationResource* animation);
		void									SetAnimState(AnimationState const& value);
		void									SetRootMotion(bool rootMotion);

		INLINE AnimationState const&			GetAnimState() const { return mAnimState; }
		INLINE bool								IsRootMotion() const { return mRootMotion; }

		INLINE bool								IsExtrapolateValues() const { return mExtrapolateValues; }
		void									SetExtrapolateValues(bool extrapolate);

		virtual void							OnDeserialized() override;
	protected:
		void                                    LoadAnimationCache();

		DField()
		AnimationState							mAnimState;

	private:

		void									CreateAnimationToJointIndexMap();

		void									UpdateSkeletalMeshSkeleton(float deltaTime);
		
		void									UpdatePropertyValues(float deltaTime);
		void									UpdatePropertyValue(D_ECS_COMP::ComponentBase* targetComponent, rttr::property prop, Track const& propertyAnimationData) const;

		DField(Serialize)
		bool									mRootMotion;

		DField(Serialize)
		bool									mExtrapolateValues;

		DField(Serialize)
		D_RESOURCE::ResourceRef<AnimationResource> mAnimation;

		D_CONTAINERS::DUnorderedMap<int, int>	mAnimationJointIndexMap; // Animation joint index to skeleton joint index

		D_CORE::Uuid							mMeshId;

		D_CONTAINERS::DUnorderedMap<D_CORE::StringId, rttr::type> mComponentNameTypeIdMap;
	};

}

File_AnimationComponent_GENERATED
