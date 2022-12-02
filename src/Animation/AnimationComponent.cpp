#include "pch.hpp"
#include "AnimationComponent.hpp"

#include <Scene/EntityComponentSystem/Components/SkeletalMeshRendererComponent.hpp>

#include <imgui.h>

using namespace D_ECS_COMP;

namespace Darius::Animation
{
	D_H_COMP_DEF(AnimationComponent);

	AnimationComponent::AnimationComponent() :
		ComponentBase(),
		mMeshId(),
		mRootMotion(false)
	{ }

	AnimationComponent::AnimationComponent(D_CORE::Uuid uuid) :
		ComponentBase(uuid),
		mMeshId(),
		mRootMotion(false)
	{ }

	void AnimationComponent::SetAnimation(ResourceHandle handle)
	{
		mChangeSignal();
		_SetAnimation(handle);
	}

	void AnimationComponent::_SetAnimation(ResourceHandle handle)
	{
		mAnimationResource = D_RESOURCE::GetResource<AnimationResource>(handle, *this);
	}

	static inline float ToFloat(const int8_t x) { return Math::Max(x / 127.0f, -1.0f); }
	static inline float ToFloat(const uint8_t x) { return x / 255.0f; }
	static inline float ToFloat(const int16_t x) { return Math::Max(x / 32767.0f, -1.0f); }
	static inline float ToFloat(const uint16_t x) { return x / 65535.0f; }

	static inline void Lerp3(float* Dest, const float* Key1, const float* Key2, float T)
	{
		Dest[0] = Math::Lerp(Key1[0], Key2[0], T);
		Dest[1] = Math::Lerp(Key1[1], Key2[1], T);
		Dest[2] = Math::Lerp(Key1[2], Key2[2], T);
	}

	static inline void AnimLerp(float* Dest, const float* Key1, const float* Key2, float T)
	{
		*Dest = Math::Lerp(*Key1, *Key2, T);
	}

	template <typename T>
	static inline Quaternion ToQuat(const T* rot)
	{
		return (Quaternion)Vector4(ToFloat(rot[0]), ToFloat(rot[1]), ToFloat(rot[2]), ToFloat(rot[3]));
	}

	static inline Quaternion ToQuat(const float* rot)
	{
		return Quaternion(XMLoadFloat4((const XMFLOAT4*)rot));
	}

	static inline void Slerp(float* Dest, const void* Key1, const void* Key2, float T, uint32_t Format)
	{
		switch (Format)
		{
		case AnimationCurve::kSNorm8:
		{
			const int8_t* key1 = (const int8_t*)Key1;
			const int8_t* key2 = (const int8_t*)Key2;
			XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
			break;
		}
		case AnimationCurve::kUNorm8:
		{
			const uint8_t* key1 = (const uint8_t*)Key1;
			const uint8_t* key2 = (const uint8_t*)Key2;
			XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
			break;
		}
		case AnimationCurve::kSNorm16:
		{
			const int16_t* key1 = (const int16_t*)Key1;
			const int16_t* key2 = (const int16_t*)Key2;
			XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
			break;
		}
		case AnimationCurve::kUNorm16:
		{
			const uint16_t* key1 = (const uint16_t*)Key1;
			const uint16_t* key2 = (const uint16_t*)Key2;
			XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
			break;
		}
		case AnimationCurve::kFloat:
		{
			const float* key1 = (const float*)Key1;
			const float* key2 = (const float*)Key2;
			XMStoreFloat4((XMFLOAT4*)Dest, (FXMVECTOR)Math::Slerp(ToQuat(key1), ToQuat(key2), T));
			break;
		}
		default:
			D_ASSERT(0, "Unexpected animation key frame data format");
			break;
		}
	}

#ifdef _DEBUG
	bool AnimationComponent::DrawDetails(float params[])
	{
		bool valueChanged = false;

		if (ImGui::BeginTable("##componentLayout", 2, ImGuiTableFlags_BordersInnerV))
		{
			ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Animation Clip");
			ImGui::TableSetColumnIndex(1);
			// Mesh selection
			D_H_RESOURCE_SELECTION_DRAW(AnimationResource, mAnimationResource, "Select Animation", SetAnimation);

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Root Motion");
			ImGui::TableSetColumnIndex(1);
			if (ImGui::Checkbox("##RootMotion", &mRootMotion))
				valueChanged = true;

			ImGui::EndTable();
		}

		if (ImGui::Button("Play"))
			mAnimState.State = AnimationState::kLooping;

		return valueChanged;
	}
#endif

	void AnimationComponent::Update(float deltaTime)
	{
		if (!IsActive() || !mAnimationResource.IsValid() || !GetGameObject()->HasComponent<D_ECS_COMP::SkeletalMeshRendererComponent>())
			return;

		SkeletalMeshRendererComponent* skeletalMesh = GetGameObject()->GetComponent<SkeletalMeshRendererComponent>();
		auto meshId = skeletalMesh->GetUuid();

		// If the skeletal mesh we are animating is changed, we have to update our indices
		if (meshId != mMeshId)
			CreateAnimationToJointIndexMap();


		// Animating skeleton
		if (mAnimState.State == AnimationState::kStopped)
			return;

		mAnimState.Time += deltaTime;

		const AnimationResource& animResource = *mAnimationResource.Get();

		const AnimationLayer& animation = animResource.GetAnimationData();

		if (mAnimState.State == AnimationState::kLooping)
		{
			mAnimState.Time = fmodf(mAnimState.Time, animation.Duration);
		}
		else if (mAnimState.Time > animation.Duration)
		{
			mAnimState.Time = 0.0f;
			mAnimState.State = AnimationState::kStopped;
		}

		// Update animation nodes
		for (AnimationCurve const& curve : animResource.GetCurvesData())
		{
			if (!mAnimationJointIndexMap.contains(curve.TargetNode))
				continue;

			auto curveTime = mAnimState.Time - curve.StartTime;
			auto upperIter = curve.KeyframeTimeMap.upper_bound(curveTime);
			auto key2pair = *upperIter;
			--upperIter;
			auto key1pair = *upperIter;

			const float progress = curveTime;
			const float key1Time = key1pair.first;
			const float lerpT = (progress - key1Time) / (key2pair.first - key1Time);

			const size_t stride = curve.KeyFrameStride * 4;
			const uint8_t* key1 = animResource.GetKeyframes().data() + curve.KeyFrameOffset + stride * key1pair.second;

			const uint8_t* key2 = key1 + stride;

			Mesh::SkeletonJoint& node = skeletalMesh->GetSkeleton()[mAnimationJointIndexMap[curve.TargetNode]];

			switch (curve.TargetPath)
			{
			case AnimationCurve::kTranslation:
				D_ASSERT(curve.KeyFrameFormat == AnimationCurve::kFloat);
				AnimLerp((float*)&node.Xform + 12 + curve.ChannelIndex, (const float*)key1, (const float*)key2, lerpT);
				break;
			case AnimationCurve::kRotation:
				node.StaleMatrix = true;
				AnimLerp((float*)&node.Rotation + curve.ChannelIndex, (const float*)key1, (const float*)key2, lerpT);
				break;
			case AnimationCurve::kScale:
				D_ASSERT(curve.KeyFrameFormat == AnimationCurve::kFloat);
				node.StaleMatrix = true;
				AnimLerp((float*)&node.Scale + curve.ChannelIndex, (const float*)key1, (const float*)key2, lerpT);
				break;
			default:
			case AnimationCurve::kWeights:
				D_ASSERT(0, "Unhandled blend shape weights in animation");
				break;
			}
		}

		if (!mRootMotion)
		{
			auto root = skeletalMesh->GetSkeletonRoot();
			root->Rotation = { 0.f, 0.f, 0.f };
			root->Scale = { 1.f, 1.f, 1.f };
			root->Xform.SetW({ 0.f, 0.f, 0.f, 1.f });
		}
	}

	void AnimationComponent::CreateAnimationToJointIndexMap()
	{
		if (!mAnimationResource.IsValid())
			return;

		if (!GetGameObject()->HasComponent<SkeletalMeshRendererComponent>())
			return;

		SkeletalMeshRendererComponent const* skeletalMeshRes = GetGameObject()->GetComponent<SkeletalMeshRendererComponent>();

		mMeshId = skeletalMeshRes->GetUuid();

		auto const& anmimationNameMap = mAnimationResource->GetSkeletonNameIndexMap();

		mAnimationJointIndexMap.clear();

		auto skeleton = skeletalMeshRes->GetMeshResource()->GetSkeleton();
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

	void AnimationComponent::Serialize(Json& j) const
	{
		if (mAnimationResource.IsValid())
			D_CORE::to_json(j["Animation"], mAnimationResource->GetUuid());

		j["RootMotion"] = mRootMotion;
	}

	void AnimationComponent::Deserialize(Json const& j)
	{
		auto go = GetGameObject();

		if (j.contains("Animation"))
		{
			Uuid animUuid;
			D_CORE::from_json(j["Animation"], animUuid);

			_SetAnimation(*D_RESOURCE::GetResource<AnimationResource>(animUuid, *go));
		}

		if (j.contains("RootMotion"))
		{
			mRootMotion = j["RootMotion"];
		}
	}

}

