#include "Editor/pch.hpp"
#include "PropertyAnimationCurve.hpp"

#include <Utils/Assert.hpp>

using namespace D_ANIMATION;
using namespace D_MATH;

namespace Darius::Editor::Gui::Components
{
	BasePropertyCurveEdit::BasePropertyCurveEdit(D_ANIMATION::Track* track, UINT framesPerSecond) :
		ImCurveEdit::Delegate(),
		std::enable_shared_from_this<BasePropertyCurveEdit>(),
		mTrack(track),
		mFramesPerSecond(framesPerSecond)
	{

		D_ASSERT(mTrack);
		D_ASSERT(mFramesPerSecond > 0);

	}

	void BasePropertyCurveEdit::SortValues(size_t curveIndex)
	{
		auto& keyframes = mTrack->GetKeyframes();
		std::sort(keyframes.begin(), keyframes.end(), [](Keyframe& a, Keyframe& b)
			{
				return a.Time < b.Time;
			});
		ReconstructPoints();
	}

	ImCurveEdit::CurveType BasePropertyCurveEdit::GetCurveType(size_t curveIndex) const
	{
		switch (mTrack->GetInterpolationMode())
		{
		case InterpolationMode::Step:
			return ImCurveEdit::CurveDiscrete;

		case InterpolationMode::Linear:
		case InterpolationMode::Slerp:
			return ImCurveEdit::CurveLinear;

		case InterpolationMode::CatmullRomSpline:
		case InterpolationMode::HermiteSpline:
			return ImCurveEdit::CurveSmooth;

		default:
			D_ASSERT(false);
			return ImCurveEdit::CurveLinear;
		}
	}

	Vector3PropertyCurveEdit::Vector3PropertyCurveEdit(Track* track, UINT framesPerSecond) :
		BasePropertyCurveEdit(track, framesPerSecond)
	{

		this->ReconstructPoints();

		for (int i = 0; i < GetCurveCount(); i++)
		{
			mVisible[i] = true;
		}

	}

	bool Vector3PropertyCurveEdit::SetKeyframeValue(int frameIndex, rttr::property property, rttr::instance const& targetObject)
	{
		if (!D_VERIFY(targetObject))
			return false;

		if(!D_VERIFY(property.is_valid()))
			return false;

		auto targetValueVar = property.get_value(targetObject);
		if (!D_VERIFY(targetValueVar.is_valid()))
			return false;

		if (!D_VERIFY(targetValueVar.is_type<Vector3>()))
			return false;

		Vector3 value = targetValueVar.get_value<Vector3>();

		float keyframeTime = GetTime(frameIndex);

		bool created;
		Keyframe* kf = mTrack->FindOrCreateKeyframeByTime(keyframeTime, &created);

		bool changed = created;
		Vector3& destination = kf->GetValue<Vector3>();

		// Neither Keyframe created nor value changed
		if (!created && destination == value)
			return false;

		destination = value;

		SortValues(-1 /* Don't care param */);

		return true;
	}

	void Vector3PropertyCurveEdit::AddPoint(size_t curveIndex, ImVec2 value)
	{

		Keyframe* kf = mTrack->FindOrCreateKeyframeByTime((float)GetTime((UINT)value.x));
		auto& refValue = kf->GetValue<Vector3>();

		float* rawValue = reinterpret_cast<float*>(&refValue);

		rawValue[curveIndex] = value.y;
		for (int i = 0; i < GetCurveCount(); i++)
		{
			if (i == curveIndex)
				rawValue[i] = value.y;
			else
				rawValue[i] = 0;
		}

		SortValues(curveIndex);
	}

	void Vector3PropertyCurveEdit::ReconstructPoints()
	{
		for (int i = 0; i < GetCurveCount(); i++)
		{
			mPts[i].clear();
			mPts[i].resize(mTrack->GetKeyframesCount());
		}

		mMax = ImVec2(0.f, 0.f);
		mMin = ImVec2(0.f, 0.f);

		auto& keyframes = mTrack->GetKeyframes();
		if (keyframes.size() > 0)
		{
			mMax.x = (UINT)GetFrameIndex(keyframes[keyframes.size() - 1].Time);
			mMax.y = -FLT_MAX;
			mMin.x = (UINT)GetFrameIndex(keyframes[0].Time);
			mMin.y = FLT_MAX;
		}

		int index = 0;
		for (auto& kf : keyframes)
		{
			auto& value = kf.GetValue<Vector3>();
			float* rawValue = (float*)&value;

			UINT frameIndex = GetFrameIndex(kf.Time);

			for (int i = 0; i < GetCurveCount(); i++)
			{
				float v = rawValue[i];
				mPts[i][index] = ImVec2(frameIndex, v);

				// Deciding the max value
				if (mMin.y > v)
					mMin.y = v;
				if (mMax.y < v)
					mMax.y = v;
			}

			index++;
		}

	}

	int Vector3PropertyCurveEdit::EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
	{

		Keyframe& keyframe = mTrack->GetKeyframes()[pointIndex];

		float* kfValue = (float*)&keyframe.GetValue<Vector3>();
		kfValue[curveIndex] = value.y;
		keyframe.Time = GetTime(value.x);

		SortValues(curveIndex);

		for (int i = 0; i < GetPointCount(0); i++)
		{
			if (mPts[0][i].x == value.x)
				return i;
		}

		return pointIndex;
	}

}