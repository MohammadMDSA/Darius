#include "Editor/pch.hpp"
#include "PropertyAnimationCurve.hpp"

#include "AnimationSequencerInterfaceComponent.hpp"

#include <Utils/Assert.hpp>

using namespace D_ANIMATION;
using namespace D_MATH;

namespace Darius::Editor::Gui::Components
{
	BasePropertyCurveEdit::BasePropertyCurveEdit(AnimationSequence const* sequence, rttr::property propertyRef, UINT framesPerSecond) :
		ImCurveEdit::Delegate(),
		std::enable_shared_from_this<BasePropertyCurveEdit>(),
		mSequenceComp(sequence),
		mFramesPerSecond(framesPerSecond),
		mPropertyRef(propertyRef)
	{
		D_ASSERT(mSequenceComp);
		D_ASSERT(mFramesPerSecond > 0);
		D_ASSERT(mPropertyRef.is_valid());
	}

	void BasePropertyCurveEdit::SortValues(size_t curveIndex)
	{
		auto& keyframes = GetTrack()->GetKeyframes();
		std::sort(keyframes.begin(), keyframes.end(), [](Keyframe& a, Keyframe& b)
			{
				return a.Time < b.Time;
			});
		ReconstructPoints();
	}

	ImVec2& BasePropertyCurveEdit::GetMax()
	{
		mFrameMax.x = (float)mSequenceComp->GetFrameMax();
		return mFrameMax;
	}

	ImVec2& BasePropertyCurveEdit::GetMin()
	{
		mFrameMin.x = (float)mSequenceComp->GetFrameMin();
		return mFrameMin;
	}

	ImCurveEdit::CurveType BasePropertyCurveEdit::GetCurveType(size_t curveIndex) const
	{
		switch (GetTrack()->GetInterpolationMode())
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

	D_ANIMATION::Track* BasePropertyCurveEdit::GetTrack() const
	{
		auto result = mSequenceComp->GetSequence()->GetTrack(mPropertyRef.get_name().data());
		return const_cast<Track*>(result);
	}

	bool QuaternionPropertyCurveEdit::SetKeyframeValue(int frameIndex, rttr::property property, rttr::instance const& targetObject)
	{
		if (!D_VERIFY(targetObject))
			return false;

		if (!D_VERIFY(property.is_valid()))
			return false;

		auto targetValueVar = property.get_value(targetObject);
		if (!D_VERIFY(targetValueVar.is_valid()))
			return false;

		if (!D_VERIFY(targetValueVar.is_type<Quaternion>()))
			return false;

		Quaternion quat = targetValueVar.get_value<Quaternion>();

		float keyframeTime = GetTime(frameIndex);

		bool created;
		Keyframe* kf = GetTrack()->FindOrCreateKeyframeByTime(keyframeTime, &created);

		Vector3& dest = kf->GetValue<Vector3>();
		Vector3 euler = quat.Angles();

		if (!created && dest == euler)
			return false;

		dest = euler;
		SortValues(-1);
		return true;
	}
}