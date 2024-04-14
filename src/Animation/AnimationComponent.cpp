#include "pch.hpp"
#include "AnimationComponent.hpp"

#include <Renderer/Components/SkeletalMeshRendererComponent.hpp>

#include <rttr/type.h>

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
		mAnimation(),
		mExtrapolateValues(true)
	{
	}

	AnimationComponent::AnimationComponent(D_CORE::Uuid const& uuid) :
		ComponentBase(uuid),
		mMeshId(),
		mRootMotion(false),
		mAnimation(),
		mExtrapolateValues(true)
	{
	}

#ifdef _D_EDITOR
	bool AnimationComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		D_H_DETAILS_DRAW_BEGIN_TABLE();

		D_H_DETAILS_DRAW_PROPERTY("Animation Clip");
		D_H_RESOURCE_SELECTION_DRAW(AnimationResource, mAnimation, "Select Animation", SetAnimation);

		// Root motion
		{
			bool value = IsRootMotion();
			D_H_DETAILS_DRAW_PROPERTY("Root Motion");
			if (ImGui::Checkbox("##RootMotion", &value))
			{
				SetRootMotion(value);
				valueChanged = true;
			}

		}

		// Extrapolate values
		{
			bool value = IsExtrapolateValues();
			D_H_DETAILS_DRAW_PROPERTY("Extrapolate Values");
			if (ImGui::Checkbox("##ExtrapolateValues", &value))
			{
				SetExtrapolateValues(value);
				valueChanged = true;
			}
		}

		D_H_DETAILS_DRAW_END_TABLE();

		if (ImGui::Button("Play"))
			mAnimState.State = AnimationState::kLooping;

		return valueChanged;
	}
#endif

	void AnimationComponent::UpdateSkeletalMeshSkeleton(float deltaTime)
	{
		if (!GetGameObject()->HasComponent<D_RENDERER::SkeletalMeshRendererComponent>())
			return;

		D_RENDERER::SkeletalMeshRendererComponent* skeletalMesh = GetGameObject()->GetComponent<D_RENDERER::SkeletalMeshRendererComponent>();
		auto meshId = skeletalMesh->GetUuid();

		// If the skeletal mesh we are animating is changed, we have to update our indices
		if (meshId != mMeshId)
			CreateAnimationToJointIndexMap();


		AnimationResource const& animResource = *mAnimation.Get();

		Sequence const& animation = animResource.GetSkeletalAnimationSequence();

		D_CONTAINERS::DVector<Track> const& tracks = animation.GetTracks();

		// Update animation nodes
		for (int i = 0; i < tracks.size(); i++)
		{
			// Since every bone has translation, rotation, and scale tracks, the index of the bone
			// associated to a track is the index of the track divided by 3
			Mesh::SkeletonJoint& node = skeletalMesh->GetSkeleton()[i / 3];

			Track const& track = tracks[i];

			switch (i % 3)
			{
			case 0: // Translation
			{
				auto value = track.Evaluate<Vector4>(mAnimState.Time, true);
				if (value.has_value())
					node.Xform.SetW(value.value());
				break;
			}
			case 1: // Scale
			{
				node.StaleMatrix = true;
				auto value = track.Evaluate<Vector4>(mAnimState.Time, true);
				if (value.has_value())
					node.Scale = (DirectX::XMFLOAT3)Vector3(value.value());
				break;
			}
			case 2: // Rotation
			default:
			{
				node.StaleMatrix = true;
				auto value = track.Evaluate<Vector4>(mAnimState.Time, true);
				if (value.has_value())
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

	void AnimationComponent::UpdatePropertyValue(D_ECS_COMP::ComponentBase* targetComponent, rttr::property prop, Track const& propertyAnimationData) const
	{
		using TypeId = rttr::type::type_id;
		static const TypeId boolType = rttr::type::get<bool>().get_id();
		static const TypeId intType = rttr::type::get<int>().get_id();
		static const TypeId uintType = rttr::type::get<UINT>().get_id();
		static const TypeId shortType = rttr::type::get<short>().get_id();
		static const TypeId ushortType = rttr::type::get<USHORT>().get_id();
		static const TypeId	longType = rttr::type::get<long>().get_id();
		static const TypeId uLongType = rttr::type::get<ULONG>().get_id();
		static const TypeId vector2Type = rttr::type::get<Vector2>().get_id();
		static const TypeId vector3Type = rttr::type::get<Vector3>().get_id();
		static const TypeId vector4Type = rttr::type::get<Vector4>().get_id();
		static const TypeId colorType = rttr::type::get<Color>().get_id();
		static const TypeId quaternionType = rttr::type::get<Quaternion>().get_id();

		rttr::type::type_id propTypeId = prop.get_type().get_id();

#define SetPropertyValue(type) \
std::optional<type> optValue = propertyAnimationData.Evaluate<type>(mAnimState.Time, IsExtrapolateValues()); \
if(!optValue.has_value()) \
	return; \
if(!prop.set_value(*targetComponent, optValue.value())) \
{ \
	D_LOG_WARN("Failed to set value for animated property " + prop.get_name().to_string() + " on component " + targetComponent->GetComponentName() + " with type " + D_NAMEOF(type)); \
}

		if (propTypeId == boolType)
		{
			SetPropertyValue(bool);
		}
		else if (propTypeId == intType)
		{
			SetPropertyValue(int);
		}
		else if (propTypeId == uintType)
		{
			SetPropertyValue(UINT);
		}
		else if (propTypeId == shortType)
		{
			SetPropertyValue(short);
		}
		else if (propTypeId == ushortType)
		{
			SetPropertyValue(USHORT);
		}
		else if (propTypeId == longType)
		{
			SetPropertyValue(long);
		}
		else if (propTypeId == uLongType)
		{
			SetPropertyValue(ULONG);
		}
		else if (propTypeId == vector2Type)
		{
			SetPropertyValue(Vector2);
		}
		else if (propTypeId == vector3Type)
		{
			//SetPropertyValue(Vector3);
			std::optional<Vector3> optValue = propertyAnimationData.Evaluate<Vector3>(mAnimState.Time, IsExtrapolateValues());
			if (!optValue.has_value())
				return;

			if (!prop.set_value(*targetComponent, optValue.value()))
			{
				D_LOG_WARN("Failed to set value for animated property " + prop.get_name().to_string() + " on component " + targetComponent->GetComponentName() + " with type " + D_NAMEOF(type));
			}
		}
		else if (propTypeId == vector4Type)
		{
			SetPropertyValue(Vector4);
		}
		else if (propTypeId == colorType)
		{
			SetPropertyValue(Color);
		}
		else if (propTypeId == quaternionType)
		{
			std::optional<Vector3> optValue = propertyAnimationData.Evaluate<Vector3>(mAnimState.Time, IsExtrapolateValues());
			if (!optValue.has_value())
				return;
			
			if (!prop.set_value(*targetComponent, Quaternion(optValue->GetX(), optValue->GetY(), optValue->GetZ())))
			{
				D_LOG_WARN("Failed to set value for animated property " + prop.get_name().to_string() + " on component " + targetComponent->GetComponentName() + " with type " + D_NAMEOF(type));
			}
		}
		else
		{
			D_ASSERT_M(false, "Unsopported property type for animation.");
		}

#undef SetPropertyValue

	}

	void AnimationComponent::UpdatePropertyValues(float deltaTime)
	{

		// For each component in the animation
		for (auto& componentAnimation : mAnimation->GetComponentAnimationData())
		{
			auto componentTypeResult = mComponentNameTypeIdMap.find(componentAnimation.ComponentName);
			if (componentTypeResult == mComponentNameTypeIdMap.end())
			{
				// Component cache not found
				D_ASSERT_M(false, "Bad component name type id map");
				continue;
			}

			// Fetching the target component
			ComponentBase* targetComponent = GetGameObject()->GetComponent(componentAnimation.ComponentName);
			if (!targetComponent)
			{
				D_LOG_WARN("Component " + componentAnimation.ComponentName + " has animated properties but does not exist on the target gameobject.");
				continue;
			}

			auto const& tracks = componentAnimation.AnimationSequence.GetTracks();

			// For each animated property (track) in the component
			for (auto& [propertyName, trackIndex] : componentAnimation.AnimationSequence.GetNameIndexMapping())
			{

				rttr::property prop = componentTypeResult->second.get_property(propertyName);

				if (!prop.is_valid())
				{
					D_LOG_WARN("Property " + propertyName + " animated property was not found in component properties.");
					continue;
				}

				D_ASSERT(trackIndex < tracks.size());

				UpdatePropertyValue(targetComponent, prop, tracks[trackIndex]);
			}
		}

	}

	void AnimationComponent::Update(float deltaTime)
	{
		if (!IsActive() || !mAnimation.IsValid())
			return;

		// Update animation state
		if (mAnimState.State == AnimationState::kStopped)
			return;

		mAnimState.Time += deltaTime;

		if (mAnimState.State == AnimationState::kLooping)
		{
			mAnimState.Time = fmodf(mAnimState.Time, mAnimation->GetDuration());
		}
		else if (mAnimState.Time > mAnimation->GetDuration())
		{
			mAnimState.Time = 0.0f;
			mAnimState.State = AnimationState::kStopped;
		}


		if (mAnimation->IsSkeletalAnimation())
		{
			UpdateSkeletalMeshSkeleton(deltaTime);
			return;
		}

		UpdatePropertyValues(deltaTime);
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

		bool animValid = mAnimation.IsValid();

		if (animValid && mAnimation->IsLoaded())
			LoadAnimationCache();
		else if (animValid)
			D_RESOURCE_LOADER::LoadResourceAsync(animation, [&](auto resource)
				{
					LoadAnimationCache();
				}, true);

		mChangeSignal(this);
	}

	void AnimationComponent::LoadAnimationCache()
	{
		mComponentNameTypeIdMap.clear();

		if (!mAnimation.IsValid() || mAnimation->IsSkeletalAnimation())
			return;

		for (auto& componentSequence : mAnimation->GetComponentAnimationData())
		{
			auto componentEntity = D_WORLD::GetComponentEntity(componentSequence.ComponentName);
			mComponentNameTypeIdMap.emplace(componentSequence.ComponentName, D_WORLD::GetComponentReflectionTypeByComponentEntity(componentEntity));
		}
	}

	void AnimationComponent::SetAnimState(AnimationState const& value)
	{
		if (value == mAnimState)
			return;

		mAnimState = value;

		mChangeSignal(this);
	}

	void AnimationComponent::SetRootMotion(bool rootMotion)
	{
		if (mRootMotion == rootMotion)
			return;

		mRootMotion = rootMotion;

		mChangeSignal(this);
	}

	void AnimationComponent::SetExtrapolateValues(bool extrapolate)
	{
		if (mExtrapolateValues == extrapolate)
			return;

		mExtrapolateValues = extrapolate;

		mChangeSignal(this);
	}

	void AnimationComponent::OnDeserialized()
	{
		LoadAnimationCache();
	}
}

