#include "Editor/pch.hpp"
#include "AnimationSequencerInterfaceComponent.hpp"

#include <Animation/AnimationResource.hpp>
#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include <rttr/type.h>
#include <imgui_internal.h>
#include <Libs/FontIcon/IconsFontAwesome6.h>

using namespace D_ANIMATION;

namespace Darius::Editor::Gui::Components
{
	AnimationSequence::AnimationSequence() :
		mFrameMin(0),
		mFrameMax(60),
		mExpanded(false),
		mAnimationSequence(nullptr)
	{
	}

	int AnimationSequence::GetFrameMin() const {
		return mFrameMin;
	}

	int AnimationSequence::GetFrameMax() const {
		return mFrameMax;
	}
	int AnimationSequence::GetItemCount() const
	{
		return (int)mPropertyCurves.size();
	}

	int AnimationSequence::GetItemTypeCount() const
	{
		return (int)(mAllProperties.size() - mPropertyCurves.size());
	}

	const char* AnimationSequence::GetItemTypeName(int typeIndex) const
	{
		int totalIndex = -1;
		int mCurveIndex = 0;

		for (int i = 0; i < mAllProperties.size(); i++)
		{
			if (mPropertyCurves.size() > mCurveIndex && mAllProperties[i] == mPropertyCurves[mCurveIndex].PropertyRef)
			{
				mCurveIndex++;
			}
			else
			{
				totalIndex++;
			}
			
			if (totalIndex == typeIndex)
				return mAllProperties[i].get_name().data();
		}

		return "";
	}

	const char* AnimationSequence::GetItemLabel(int index) const
	{
		return mPropertyCurves[index].PropertyRef.get_name().data();
	}

	void AnimationSequence::Get(int index, int** start, int** end, int* type, unsigned int* color)
	{
		SequenceItem& item = mPropertyCurves[index];

		if (color)
			*color = 0xFFAA8080; // same color for everyone, return color based on type
		if (start)
			*start = &item.FrameStart;
		if (end)
			*end = &item.FrameEnd;
		if (type)
			*type = item.Type;
	}

	void AnimationSequence::Add(int type)
	{
		rttr::property prop = mAllProperties[type];
		UINT trackIndex = mAnimationSequence->AddTrack(prop.get_name().data(), {});

		Track* track = const_cast<Track*>(&mAnimationSequence->GetTracks()[trackIndex]);
		mAnimationResource->MakeDiskDirty();
		mAnimationResource->MakeGpuDirty();

		UINT fps = mAnimationResource->GetFramesPerSecond();

		mPropertyCurves.push_back(SequenceItem{ type, std::make_shared<Vector3PropertyCurveEdit>(track, fps), mAllProperties[type], (int)track->GetFirstFrame(fps), (int)track->GetLastFrame(fps), true});
		std::sort(mPropertyCurves.begin(), mPropertyCurves.end(), [](SequenceItem const& a, SequenceItem const& b) { return a.PropertyRef.get_name() < b.PropertyRef.get_name(); });
	};

	void AnimationSequence::Del(int index)
	{

		auto& sequenceItem = mPropertyCurves[index];
		bool removed = mAnimationSequence->RemoveTrack(sequenceItem.PropertyRef.get_name().data());

		mPropertyCurves.erase(mPropertyCurves.begin() + index);
		std::sort(mPropertyCurves.begin(), mPropertyCurves.end(), [](SequenceItem const& a, SequenceItem const& b) { return a.PropertyRef.get_name() < b.PropertyRef.get_name(); });

		mAnimationResource->MakeDiskDirty();
		mAnimationResource->MakeGpuDirty();
	}

	void AnimationSequence::Duplicate(int index)
	{
	}

	size_t AnimationSequence::GetCustomHeight(int index)
	{
		return mPropertyCurves[index].Expanded ? 300 : 0;
	}

	void AnimationSequence::DoubleClick(int index)
	{
		if (mPropertyCurves[index].Expanded)
		{
			mPropertyCurves[index].Expanded = false;
			return;
		}
		for (auto& item : mPropertyCurves)
			item.Expanded = false;

		mPropertyCurves[index].Expanded = !mPropertyCurves[index].Expanded;
	}

	void AnimationSequence::CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
	{
		auto& item = mPropertyCurves[index];

		draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
		for (int i = 0; i < item.Curve->GetCurveCount(); i++)
		{
			ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
			ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
			draw_list->AddText(pta, item.Curve->IsVisible(i) ? 0xFFFFFFFF : 0x80FFFFFF, item.Curve->GetCurveName(i));
			if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
				item.Curve->SetVisible(i, !item.Curve->IsVisible(i));
		}
		draw_list->PopClipRect();

		ImGui::SetCursorScreenPos(rc.Min);
		
		if (ImCurveEdit::Edit(*item.Curve, rc.Max - rc.Min, 137 + index, &clippingRect))
		{
			mAnimationResource->MakeGpuDirty();
			mAnimationResource->MakeDiskDirty();
		}
	}

	void AnimationSequence::CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
	{
		draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);

		auto& item = mPropertyCurves[index];

		for (int i = 0; i < item.Curve->GetCurveCount(); i++)
		{
			for (int j = 0; j < item.Curve->GetPointCount(i); j++)
			{
				auto p = item.Curve->GetPoints(i)[j].x;
				float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
				float x = D_MATH::Lerp(rc.Min.x, rc.Max.x, r);
				draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
			}
		}
		draw_list->PopClipRect();
	}

	void AnimationSequence::Initialize(ComponentBase* referenceComponent, D_ANIMATION::AnimationResource* animationRes, D_ANIMATION::Sequence* keyframeSequence)
	{
		if (referenceComponent == mReferenceComponent)
			return;

		mAllProperties.clear();

		mReferenceComponent = referenceComponent;

		if (mReferenceComponent)
		{
			mComponentDisplayName = mReferenceComponent->GetDisplayName();
			mCollapsedDisplayName = mComponentDisplayName + ":\t%d Frames / %d entries";
		}
		else
		{
			mComponentDisplayName = "";
			mCollapsedDisplayName = "";
		}

		if (mReferenceComponent)
		{
			for (auto& prop : referenceComponent->get_type().get_properties())
			{
				if (prop.get_metadata("ANIMATE"))
					mAllProperties.push_back(prop);
			}

			std::sort(mAllProperties.begin(), mAllProperties.end(), [](rttr::property const& a, rttr::property const& b)
				{
					return a.get_name() < b.get_name();
				});

		}

		mAnimationResource = animationRes;
		mAnimationSequence = keyframeSequence;

		InitializeSequenceItems();
	}

	char const* AnimationSequence::GetCollapseFmt() const
	{
		return mCollapsedDisplayName.c_str();
	}

	void AnimationSequence::InitializeSequenceItems()
	{

		for (int propIndex = 0; propIndex < mAllProperties.size(); propIndex++)
		{
			auto const& prop = mAllProperties[propIndex];
			auto propName = prop.get_name().data();
			auto track = const_cast<Track*>(mAnimationSequence->GetTrack(propName));

			if (!track)
				continue;

			UINT fps = mAnimationResource->GetFramesPerSecond();
			mPropertyCurves.push_back({ propIndex, std::make_shared<Vector3PropertyCurveEdit>(track, fps), prop, (int)track->GetFirstFrame(fps), (int)track->GetLastFrame(fps), true });
		}

		std::sort(mPropertyCurves.begin(), mPropertyCurves.end(), [](SequenceItem const& a, SequenceItem const& b) { return a.PropertyRef.get_name() < b.PropertyRef.get_name(); });
	}
}