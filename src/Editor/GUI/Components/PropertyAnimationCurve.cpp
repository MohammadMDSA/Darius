#include "Editor/pch.hpp"
#include "PropertyAnimationCurve.hpp"

#include <Utils/Assert.hpp>

using namespace D_ANIMATION;
using namespace D_MATH;

namespace Darius::Editor::Gui::Components
{
	Vector3PropertyCurveEdit::Vector3PropertyCurveEdit(Track* track, UINT framesPerSecond) :
		ImCurveEdit::Delegate(),
		std::enable_shared_from_this<Vector3PropertyCurveEdit>(),
		mTrack(track),
		mFramesPerSecond(framesPerSecond)
	{
		D_ASSERT(mTrack);
		D_ASSERT(mFramesPerSecond > 0);

		mVisible[0] = mVisible[1] = mVisible[2] = true;

		ReconstructPoints();
	}

	ImCurveEdit::CurveType Vector3PropertyCurveEdit::GetCurveType(size_t curveIndex) const
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

	void Vector3PropertyCurveEdit::AddPoint(size_t curveIndex, ImVec2 value)
	{
		if (curveIndex > 2)
			return;

		float kfTime = GetTime(static_cast<UINT>(value.x));

		Keyframe* kf = mTrack->FindOrCreateKeyframeByTime(kfTime);

		if (curveIndex == 0)
			kf->GetValue<Vector3>().SetX(value.y);

		else if (curveIndex == 1)
			kf->GetValue<Vector3>().SetY(value.y);

		else if (curveIndex == 2)
			kf->GetValue<Vector3>().SetZ(value.y);
		
		SortValues(curveIndex);
	}

	void Vector3PropertyCurveEdit::SortValues(size_t curveIndex)
	{
		auto& keyframes = mTrack->GetKeyframes();
		std::sort(keyframes.begin(), keyframes.end(), [](Keyframe& a, Keyframe& b)
			{
				return a.Time < b.Time;
			});
		ReconstructPoints();
	}

	void Vector3PropertyCurveEdit::ReconstructPoints()
	{
		for (int i = 0; i < GetCurveCount(); i++)
		{
			mPts[i].clear();
			mPts[i].resize(mTrack->GetKeyframesCount());
		}

		ImVec2 maxPoint(0.f, 0.f);
		ImVec2 minPoint(0.f, 0.f);

		auto& keyframes = mTrack->GetKeyframes();
		if (keyframes.size() > 0)
		{
			maxPoint.x = keyframes[keyframes.size() - 1].Time;
			minPoint.x = keyframes[0].Time;
		}

		int index = 0;
		for (auto& kf : mTrack->GetKeyframes())
		{
			auto& value = kf.GetValue<Vector3>();
			float* rawValue = (float*)&value;

			for (int i = 0; i < GetCurveCount(); i++)
			{
				float v = rawValue[i];
				mPts[i][index] = ImVec2((float)GetFrameIndex(kf.Time), v);

				// Deciding the max value
				if (minPoint.y > v)
					minPoint.y = v;
				if (maxPoint.y < v)
					maxPoint.y = v;
			}

			index++;
		}
	}

	int Vector3PropertyCurveEdit::EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
	{

		Keyframe& keyframe = mTrack->GetKeyframes()[pointIndex];

		float* kfValue = (float*)&keyframe.GetValue<Vector3>();
		kfValue[curveIndex] = value.y;

		SortValues(curveIndex);

		for (int i = 0; i < GetPointCount(0); i++)
		{
			if (mPts[0][i].x == value.x)
				return i;
		}

		return pointIndex;
	}

}