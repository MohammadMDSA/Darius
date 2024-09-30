#include "pch.hpp"
#include "AnimationResource.hpp"

#include <Core/Serialization/TypeSerializer.hpp>

#define FBXSDK_SHARED

#include <fbxsdk.h>
#include <fbxsdk/fileio/fbxiosettings.h>

#include "AnimationResource.sgenerated.hpp"

using namespace D_CONTAINERS;
using namespace D_FILE;
using namespace D_MATH;
using namespace D_RESOURCE;

namespace Darius::Animation
{

	D_CH_RESOURCE_DEF(AnimationResource);

	AnimationResource::AnimationResource(D_CORE::Uuid const& uuid, std::wstring const& path, std::wstring const& name, D_RESOURCE::DResourceId id, D_RESOURCE::Resource* parent, bool isDefault) :
		Resource(uuid, path, name, id, parent, isDefault),
		mSkeletalAnimationSequence(),
		mSkeletalAnimation(false),
		mFramesPerSecond(60u)
	{
		struct StaticInitializer
		{
			StaticInitializer()
			{
				D_SERIALIZATION::RegisterSerializer<ComponentAnimationData>(
					[](ComponentAnimationData const& data, D_SERIALIZATION::Json& j)
					{
						j["ComponentName"] = data.ComponentName.string();
						D_SERIALIZATION::Serialize(data.AnimationSequence, j["AnimationSequence"]);
					},
					[](ComponentAnimationData& data, D_SERIALIZATION::Json const& j)
					{
						data.ComponentName = D_CORE::StringId(j["ComponentName"].get<std::string>().c_str());
						D_SERIALIZATION::Deserialize(data.AnimationSequence, j["AnimationSequence"]);
					}
				);
			}
		};
	
		static StaticInitializer _Initializer;
		(_Initializer);
	}

	D_RESOURCE::SubResourceConstructionData AnimationResource::CanConstructFrom(ResourceType type, Path const& path)
	{
		DVector<ResourceDataInFile> subRes;
		auto ext = path.extension();
		if (ext == ".anim")
		{
			ResourceDataInFile res;
			res.Name = WSTR2STR(D_FILE::GetFileName(path));
			res.Type = AnimationResource::GetResourceType();
		}
		D_RESOURCE::SubResourceConstructionData res
		{
			.SubResources = subRes
		};
		return res;
	}

	void AnimationResource::ReadResourceFromFile(D_SERIALIZATION::Json const& json, bool& dirtyDisk)
	{
		auto ext = GetPath().extension();
		if (ext == ".anim")
		{
			mSkeletalAnimation = false;
			ReadNativeAnimationFromFile(json);
			return;
		}

		dirtyDisk = false;
	}

	void AnimationResource::Unload()
	{
		EvictFromGpu();
		mSkeletonNameIndexMap.clear();
		mSkeletalAnimationSequence = Sequence();
	}

	void AnimationResource::CreateSkeletalAnimation(Sequence const& seq, D_CONTAINERS::DUnorderedMap<D_CORE::StringId, int> const& jointNameIndexMap)
	{
		mSkeletalAnimationSequence = seq;
		mSkeletonNameIndexMap = jointNameIndexMap;
		mComponentAnimation = {};
		mSkeletalAnimation = true;

		MakeDiskDirty();
		SignalChange();
	}

	void AnimationResource::SetFrameRate(float frameRate)
	{
		UINT frate = (UINT)frameRate;
		if (mFramesPerSecond == frate)
			return;

		mFramesPerSecond = frate;

		MakeDiskDirty();
		SignalChange();
	}

	void AnimationResource::ReadNativeAnimationFromFile(D_SERIALIZATION::Json const& json)
	{
		D_SERIALIZATION::Json animData;
		if (!D_FILE::ReadJsonFile(GetPath(), animData))
		{
			D_LOG_ERROR("Could not read animation data from " + GetPath().string());
			return;
		}

		D_SERIALIZATION::Deserialize(*this, animData);
	}

	bool AnimationResource::WriteResourceToFile(D_SERIALIZATION::Json& json) const
	{
		if (IsSkeletalAnimation())
		{
			D_LOG_ERROR("Unable to write skeletal animation to file " << GetPath().string());
			return false;
		}

		D_SERIALIZATION::Json animData;
		D_SERIALIZATION::Serialize(*this, animData);

		if (D_FILE::WriteJsonFile(GetPath(), animData))
		{
			D_LOG_ERROR("Unable to write animation data to " + GetPath().string());
			return false;
		}
		return true;
	}

	float AnimationResource::GetStartTime() const
	{
		if (IsSkeletalAnimation())
			return mSkeletalAnimationSequence.GetStartTime();

		if (mComponentAnimation.size() <= 0)
			return 0.f;

		// Check for earliest sequence
		auto earliestAnim = std::min_element(mComponentAnimation.begin(), mComponentAnimation.end(), [](auto const& el1, auto const& el2) { return el1.AnimationSequence.GetStartTime() < el2.AnimationSequence.GetStartTime(); });

		return earliestAnim->AnimationSequence.GetStartTime();
	}

	float AnimationResource::GetEndTime() const
	{
		if (IsSkeletalAnimation())
			return mSkeletalAnimationSequence.GetEndTime();

		if (mComponentAnimation.size() <= 0)
			return 0.f;

		// Check for lates sequence

		auto latestAnim = std::max_element(mComponentAnimation.begin(), mComponentAnimation.end(), [](auto const& el1, auto const& el2) { return el1.AnimationSequence.GetStartTime() < el2.AnimationSequence.GetStartTime(); });

		return latestAnim->AnimationSequence.GetEndTime();
	}

}
