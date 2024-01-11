#include "pch.hpp"
#include "AnimationCommon.hpp"

#include "AnimationCommon.sgenerated.hpp"

using namespace D_MATH;
using namespace D_SERIALIZATION;

namespace Darius::Animation
{

	Keyframe* Track::AddKeyframe(Keyframe const& keyframe, int index)
	{
		if (index < 0)
		{
			mKeyframes.push_back(keyframe);
			return &mKeyframes.back();
		}

		mKeyframes.insert(mKeyframes.begin() + index, keyframe);
		return &mKeyframes[index];
	}

	float Track::GetStartTime() const
	{
		if (!mKeyframes.empty())
			return mKeyframes[0].Time;

		return 0.f;
	}

	float Track::GetEndTime() const
	{
		if (!mKeyframes.empty())
			return mKeyframes[mKeyframes.size() - 1].Time;

		return 0.f;
	}

	Keyframe const* Track::FindKeyframeByTime(float time) const
	{
		for (auto& kf : mKeyframes)
		{
			// If found, return it
			if (kf.Time == time)
				return &kf;

			// If current keyframe has greater time, desired keyframe was not available
			if (kf.Time > time)
				return nullptr;
		}

		// Finished and didn't find the desired keyframe
		return nullptr;
	}

	Keyframe* Track::FindKeyframeByTime(float time)
	{
		for (auto& kf : mKeyframes)
		{
			// If found, return it
			if (kf.Time == time)
				return &kf;
			
			// If current keyframe has greater time, desired keyframe was not available
			if (kf.Time > time)
				return nullptr;
		}

		// Finished and didn't find the desired keyframe
		return nullptr;
	}

	Keyframe* Track::FindOrCreateKeyframeByTime(float time)
	{
		auto kfSize = mKeyframes.size();

		if (kfSize == 0)
		{
			return AddKeyframe({});
		}

		// Assuming keyframes are sorted by their time property
		for (int i = 0; i < kfSize; i++)
		{
			auto& kf = mKeyframes[i];
			if (kf.Time == time)
				return &kf;
			if (kf.Time > time)
				return AddKeyframe({}, i);
		}

		// If we are here, then all keyframes are happening before the given time
		return AddKeyframe({});
	}

	UINT Sequence::AddTrack(std::string const& name, Track const& track)
	{

		UINT trackIndex = (UINT)mTracks.size();
		mTracks.push_back(track);
		mTracksNameIndex[name] = trackIndex;
		mDuration = D_MATH::Max(mDuration, track.GetEndTime());

		return trackIndex;
	}
}