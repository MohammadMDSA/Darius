#include "pch.hpp"
#include "AnimationCommon.hpp"

#include "AnimationCommon.sgenerated.hpp"

using namespace D_MATH;
using namespace D_SERIALIZATION;

namespace Darius::Animation
{
	Track::Track() :
		mKeyframes(),
		mMode(InterpolationMode::Linear),
		mKeyframeDataType(KeyframeDataType::Vector3)
	{}

	Track::Track(InterpolationMode mode, KeyframeDataType dataType) :
		mKeyframes(),
		mMode(mode),
		mKeyframeDataType(dataType)
	{}

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

	Keyframe* Track::FindOrCreateKeyframeByTime(float time, bool* created)
	{
		auto kfSize = mKeyframes.size();

		if (created)
			*created = false;

		if (kfSize == 0)
		{
			if (created)
				*created = true;
			Keyframe* result = AddKeyframe({});
			result->Time = time;
			return result;
		}

		// Assuming keyframes are sorted by their time property
		for (int i = 0; i < kfSize; i++)
		{
			auto& kf = mKeyframes[i];
			if (kf.Time == time)
				return &kf;
			if (kf.Time > time)
			{
				if (created)
					*created = true;

				Keyframe* result = AddKeyframe({}, i);
				result->Time = time;
				return result;
			}
		}

		// If we are here, then all keyframes are happening before the given time
		if (created)
			*created = true;

		Keyframe* result = AddKeyframe({});
		result->Time = time;
		return result;
	}

	UINT Sequence::AddTrack(std::string const& name, Track const& track)
	{

		UINT trackIndex = (UINT)mTracks.size();
		mTracks.push_back(new Track(track));
		mTracksNameIndex[name] = trackIndex;

		return trackIndex;
	}

	bool Sequence::RemoveTrack(std::string const& name)
	{
		auto search = mTracksNameIndex.find(name);

		if (search == mTracksNameIndex.end())
			return false;

		UINT trackIdx = search->second;

		delete mTracks[trackIdx];
		mTracks.erase(mTracks.begin() + trackIdx);
		mTracksNameIndex.erase(name);
		return true;
	}

	float Sequence::GetStartTime() const
	{
		if (mTracks.size() <= 0)
			return 0.f;

		auto earliestTrack = std::min_element(mTracks.begin(), mTracks.end(), [](Track* a, Track* b)
			{
				return a->GetStartTime() < b->GetStartTime();
			});

		return (*earliestTrack)->GetStartTime();
	}

	float Sequence::GetEndTime() const
	{
		if (mTracks.size() <= 0)
			return 0.f;

		auto latestTrack = std::max_element(mTracks.begin(), mTracks.end(), [](Track const* a, Track const* b)
			{
				return a->GetEndTime() < b->GetEndTime();
			});

		return (*latestTrack)->GetEndTime();
	}

}