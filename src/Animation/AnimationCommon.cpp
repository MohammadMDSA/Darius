#include "pch.hpp"
#include "AnimationCommon.hpp"

#include "AnimationCommon.sgenerated.hpp"

using namespace D_MATH;
using namespace D_SERIALIZATION;

namespace Darius::Animation
{
	Vector4 Interpolate(InterpolationMode mode, Keyframe const& a, Keyframe const& b, Keyframe const& c, Keyframe const& d, float t, float dt)
	{
		switch (mode)
		{
		case InterpolationMode::Step:
			return b.Value;

		case InterpolationMode::Linear:
			return Lerp(b.Value, c.Value, t);

		case InterpolationMode::Slerp:
		{
			Quaternion qb(b.Value);
			Quaternion qc(c.Value);
			Quaternion qr = Slerp(qb, qc, t);
			return Vector4(qr);
		}

		case InterpolationMode::CatmullRomSpline:
		{
			// https://en.wikipedia.org/wiki/Cubic_Hermite_spline#Interpolation_on_the_unit_interval_with_matched_derivatives_at_endpoints
			// a = p[n-1], b = p[n], c = p[n+1], d = p[n+2]
			Vector4 i = -a.Value + 3.f * b.Value - 3.f * c.Value + d.Value;
			Vector4 j = 2.f * a.Value - 5.f * b.Value + 4.f * c.Value - d.Value;
			Vector4 k = -a.Value + c.Value;
			return 0.5f * ((i * t + j) * t + k) * t + b.Value;
		}

		case InterpolationMode::HermiteSpline:
		{
			// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#appendix-c-spline-interpolation
			const float t2 = t * t;
			const float t3 = t2 * t;
			return (2.f * t3 - 3.f * t2 + 1.f) * b.Value
				+ (t3 - 2.f * t2 + t) * b.OutTangent * dt
				+ (-2.f * t3 + 3.f * t2) * c.Value
				+ (t3 - t2) * c.InTangent * dt;
		}

		default:
			D_ASSERT_M(false, "Unknown interpolation mode");
			return b.Value;
		}
	}

	std::optional<Vector4> Track::Evaluate(float time, bool extrapolateLastValues) const
	{
		size_t count = mKeyframes.size();

		if (count == 0)
			return std::optional<Vector4>();

		if (time <= mKeyframes[0].Time)
			return std::optional(mKeyframes[0].Value);

		if (count == 1 || time >= mKeyframes[count - 1].Time)
		{
			if (extrapolateLastValues)
				return std::optional(mKeyframes[count - 1].Value);
			else
				return std::optional<Vector4>();
		}

		for (size_t offset = 0; offset < count; offset++)
		{
			const float tb = mKeyframes[offset].Time;
			const float tc = mKeyframes[offset + 1].Time;

			if (tb <= time && time < tc)
			{
				Keyframe const& b = mKeyframes[offset];
				Keyframe const& c = mKeyframes[offset + 1];
				Keyframe const& a = (offset > 0) ? mKeyframes[offset - 1] : b;
				Keyframe const& d = (offset < count - 2) ? mKeyframes[offset + 2] : c;
				const float dt = tc - tb;
				const float u = (time - tb) / dt;

				Vector4 y = Interpolate(mMode, a, b, c, d, u, dt);

				return std::optional(y);
			}
		}

		D_ASSERT_M(true, "Keyframes are not properly ordered by their time.");
		return std::optional<Vector4>();
	}

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

	std::optional<Vector4> Sequence::Evaluate(std::string const& name, float time, bool extrapolateLastValue)
	{
		Track const* track = GetTrack(name);
		if (!track)
			return std::optional<Vector4>();

		return track->Evaluate(time, extrapolateLastValue);
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