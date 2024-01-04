#include "pch.hpp"
#include "AnimationComponent.hpp"

#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>

#ifdef _D_EDITOR
#include <Libs/FontIcon/IconsFontAwesome6.h>
#include <ResourceManager/ResourceDragDropPayload.hpp>

#include <imgui.h>
#endif

#include "AnimationComponent.sgenerated.hpp"

using namespace D_CORE;
using namespace D_ECS_COMP;
using namespace D_GRAPHICS;
using namespace D_MATH;
using namespace D_RENDERER_GEOMETRY;
using namespace D_SERIALIZATION;

namespace Darius::Animation
{
	D_H_COMP_DEF(AnimationComponent);

	AnimationComponent::AnimationComponent() :
		ComponentBase(),
		mMeshId(),
		mRootMotion(false),
		mAnimation()
	{
	}

	AnimationComponent::AnimationComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mMeshId(),
		mRootMotion(false),
		mAnimation()
	{
	}

#ifdef _D_EDITOR
	bool AnimationComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		D_H_DETAILS_DRAW_PROPERTY("Animation Clip");
		D_H_RESOURCE_SELECTION_DRAW(AnimationResource, mAnimation, "Select Animation", SetAnimation);

		D_H_DETAILS_DRAW_PROPERTY("Root Motion");
		if (ImGui::Checkbox("##RootMotion", &mRootMotion))
			valueChanged = true;

		D_H_DETAILS_DRAW_END_TABLE();

		if (ImGui::Button("Play"))
			mAnimState.State = AnimationState::kLooping;

		return valueChanged;
	}
#endif

	void AnimationComponent::Update(float deltaTime)
	{
		if (!IsActive() || !mAnimation.IsValid() || !GetGameObject()->HasComponent<D_RENDERER::SkeletalMeshRendererComponent>())
			return;

		D_RENDERER::SkeletalMeshRendererComponent* skeletalMesh = GetGameObject()->GetComponent<D_RENDERER::SkeletalMeshRendererComponent>();
		auto meshId = skeletalMesh->GetUuid();

		// If the skeletal mesh we are animating is changed, we have to update our indices
		if (meshId != mMeshId)
			CreateAnimationToJointIndexMap();


		// Animating skeleton
		if (mAnimState.State == AnimationState::kStopped)
			return;

		mAnimState.Time += deltaTime;

		AnimationResource const& animResource = *mAnimation.Get();

		Sequence const& animation = animResource.GetAnimationSequence();

		if (mAnimState.State == AnimationState::kLooping)
		{
			mAnimState.Time = fmodf(mAnimState.Time, animation.GetDuration());
		}
		else if (mAnimState.Time > animation.GetDuration())
		{
			mAnimState.Time = 0.0f;
			mAnimState.State = AnimationState::kStopped;
		}

		D_CONTAINERS::DVector<Track> const& tracks = animation.GetTracks();

		// Update animation nodes
		for (int i = 0; i < tracks.size(); i++)
		{
			// Since every bone has translation, rotation, and scale tracks, the index of the bone
			// associated to a track is the index of the track divided by 3
			Mesh::SkeletonJoint& node = skeletalMesh->GetSkeleton()[i / 3];

			Track track = tracks[i];

			switch (i % 3)
			{
			case 0: // Translation
			{
				auto value = track.Evaluate(mAnimState.Time, true);
				if (value.has_value())
					node.Xform.SetW(value.value());
				break;
			}
			case 1: // Scale
			{
				node.StaleMatrix = true;
				auto value = track.Evaluate(mAnimState.Time, true);
				if (value.has_value())
					node.Scale = (DirectX::XMFLOAT3)Vector3(value.value());
				break;
			}
			case 2: // Rotation
			default:
			{
				node.StaleMatrix = true;
				auto value = track.Evaluate(mAnimState.Time, true);
				if(value.has_value())
				node.Rotation = (DirectX::XMFLOAT3)Vector3(value.value());
				break;
			}
			}
		}

		if (!mRootMotion)
		{
			auto root = skeletalMesh->GetSkeletonRoot();
			root->Rotation = { 0.f, 0.f, 0.f };
			root->Scale = { 1.f, 1.f, 1.f };
			root->Xform.SetW({ 0.f, 0.f, 0.f, 1.f });
		}

		skeletalMesh->SetDirty();
	}

	void AnimationComponent::CreateAnimationToJointIndexMap()
	{
		if (!mAnimation.IsValid())
			return;

		if (!GetGameObject()->HasComponent<D_RENDERER::SkeletalMeshRendererComponent>())
			return;

		D_RENDERER::SkeletalMeshRendererComponent const* skeletalMeshRes = GetGameObject()->GetComponent<D_RENDERER::SkeletalMeshRendererComponent>();

		mMeshId = skeletalMeshRes->GetUuid();

		auto const& anmimationNameMap = mAnimation->GetSkeletonNameIndexMap();

		mAnimationJointIndexMap.clear();

		auto skeleton = skeletalMeshRes->GetMesh()->GetSkeleton();
		for (auto const& joint : skeleton)
		{
			const int animIndex = anmimationNameMap.at(joint.Name);
			mAnimationJointIndexMap.insert({ animIndex,  joint.MatrixIdx });
		}

	}

	void AnimationComponent::Awake()
	{
		mAnimState.Time = 0.f;
		mAnimState.State = AnimationState::kLooping;
	}

	void AnimationComponent::SetAnimation(AnimationResource* animation)
	{
		if (mAnimation == animation)
			return;

		mAnimation = animation;

		if (mAnimation.IsValid() && !mAnimation->IsLoaded())
			D_RESOURCE_LOADER::LoadResourceAsync(animation, nullptr, true);

		mChangeSignal(this);
	}

}

