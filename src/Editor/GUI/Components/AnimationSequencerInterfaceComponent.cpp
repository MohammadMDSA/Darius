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
		mExpanded(false),
		mCurrentFrame(nullptr)
	{
	}

	int AnimationSequence::GetFrameMin() const {
		UINT min = UINT_MAX;
		for (auto const& seqItem : mPropertyCurves)
		{
			UINT curveMin = seqItem.Curve->GetStartFrameIndex();
			if (min > curveMin)
				min = curveMin;
		}

		return (int)min;
	}

	int AnimationSequence::GetFrameMax() const {
		UINT max = 0u;
		for (auto const& seqItem : mPropertyCurves)
		{
			UINT curveMax = (UINT)seqItem.Curve->GetLastFrameIndex();
			if (max < curveMax)
				max = curveMax;
		}
		return (int)max;
	}

	int AnimationSequence::GetItemCount() const
	{
		return (int)mPropertyCurves.size();
	}

	int AnimationSequence::GetItemTypeCount() const
	{
		return (int)(mAllProperties.size() - mPropertyCurves.size());
	}

	rttr::property const* AnimationSequence::GetItemTypeProperty(int typeIndex) const
	{
		int totalIndex = -1;
		int mCurveIndex = 0;

		for (int i = 0; i < mAllProperties.size(); i++)
		{
			if (mPropertyCurves.size() > mCurveIndex && mAllProperties[i] == mPropertyCurves[mCurveIndex].Curve->GetPropertyRef())
			{
				mCurveIndex++;
			}
			else
			{
				totalIndex++;
			}

			if (totalIndex == typeIndex)
				return &mAllProperties[i];
		}

		return nullptr;
	}

	const char* AnimationSequence::GetItemTypeName(int typeIndex) const
	{
		auto prop = GetItemTypeProperty(typeIndex);
		if (!prop)
			return "";

		return prop->get_name().data();
	}

	const char* AnimationSequence::GetItemLabel(int index) const
	{
		return mPropertyCurves[index].Curve->GetPropertyRef().get_name().data();
	}

	void AnimationSequence::Get(int index, int** start, int** end, int* type, unsigned int* color)
	{
		SequenceItem& item = mPropertyCurves[index];

		if (color)
			*color = 0xFFAA8080; // same color for everyone, return color based on type
		if (start)
		{
			*start = &(int&)item.Curve->GetStartFrameIndex();
		}
		if (end)
		{
			*end = &(int&)item.Curve->GetLastFrameIndex();
		}
		if (type)
			*type = item.Type;
	}

	void AnimationSequence::Add(int type)
	{
		auto sequence = const_cast<Sequence*>(GetSequence());

		auto _prop = GetItemTypeProperty(type);
		D_ASSERT(_prop);

		rttr::property prop = *_prop;
		UINT trackIndex = sequence->AddTrack(prop.get_name().data(), Track(InterpolationMode::Linear, KeyframeDataType::Vector3));

		Track& track = const_cast<Track&>(sequence->GetTracks()[trackIndex]);
		mAnimationResource->MakeDiskDirty();
		mAnimationResource->MakeGpuDirty();

		UINT fps = mAnimationResource->GetFramesPerSecond();

		mPropertyCurves.push_back(SequenceItem{ type, std::make_shared<Vector3PropertyCurveEdit>(this, prop, fps), true });
		std::sort(mPropertyCurves.begin(), mPropertyCurves.end(), [](SequenceItem const& a, SequenceItem const& b) { return a.Curve->GetPropertyRef().get_name() < b.Curve->GetPropertyRef().get_name(); });
	};

	void AnimationSequence::Del(int index)
	{

		auto& sequenceItem = mPropertyCurves[index];
		bool removed = const_cast<Sequence*>(GetSequence())->RemoveTrack(sequenceItem.Curve->GetPropertyRef().get_name().data());

		mPropertyCurves.erase(mPropertyCurves.begin() + index);
		std::sort(mPropertyCurves.begin(), mPropertyCurves.end(), [](SequenceItem const& a, SequenceItem const& b) { return a.Curve->GetPropertyRef().get_name() < b.Curve->GetPropertyRef().get_name(); });

		mAnimationResource->MakeDiskDirty();
		mAnimationResource->MakeGpuDirty();
	}

	void AnimationSequence::Duplicate(int index)
	{
		SequenceItem& seqItem = mPropertyCurves[index];

		if (seqItem.Curve->SetKeyframeValue(*mCurrentFrame, seqItem.Curve->GetPropertyRef(), rttr::instance(mReferenceComponent)))
		{
			mAnimationResource->MakeGpuDirty();
			mAnimationResource->MakeDiskDirty();
		}

	}

	size_t AnimationSequence::GetCustomHeight(int index)
	{
		return mPropertyCurves[index].Expanded ? 170 : 0;
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
				float frameMin = item.Curve->GetMin().x;
				float frameMax = item.Curve->GetMax().x;
				auto p = item.Curve->GetPoints(i)[j].x;
				float r = (p - frameMin) / float(frameMax - frameMin);
				float x = D_MATH::Lerp(rc.Min.x, rc.Max.x, r);
				draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
			}
		}
		draw_list->PopClipRect();
	}

	void AnimationSequence::Initialize(ComponentBase* referenceComponent, D_ANIMATION::AnimationResource* animationRes, int& currentFrameRef)
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
		mCurrentFrame = &currentFrameRef;

		InitializeSequenceItems();
	}

	char const* AnimationSequence::GetCollapseFmt() const
	{
		return mCollapsedDisplayName.c_str();
	}

	void AnimationSequence::InitializeSequenceItems()
	{
		auto sequence = GetSequence();

		for (int propIndex = 0; propIndex < mAllProperties.size(); propIndex++)
		{
			auto const& prop = mAllProperties[propIndex];
			auto propName = prop.get_name().data();
			auto track = const_cast<Track*>(sequence->GetTrack(propName));

			if (!track)
				continue;

			UINT fps = mAnimationResource->GetFramesPerSecond();
			mPropertyCurves.push_back({ propIndex, std::make_shared<Vector3PropertyCurveEdit>(this, prop, fps), true });
		}

		std::sort(mPropertyCurves.begin(), mPropertyCurves.end(), [](SequenceItem const& a, SequenceItem const& b) { return a.Curve->GetPropertyRef().get_name() < b.Curve->GetPropertyRef().get_name(); });
	}

	Sequence const* AnimationSequence::GetSequence() const
	{
		auto const& compsData = mAnimationResource->GetComponentAnimationData();
		auto searchRes = std::find_if(compsData.begin(), compsData.end(), [compName = mReferenceComponent->GetComponentName()](ComponentAnimationData const& el)
			{
				return el.ComponentName == compName;
			});

		if (searchRes == compsData.end())
			return nullptr;

		return &(searchRes->AnimationSequence);
	}
}