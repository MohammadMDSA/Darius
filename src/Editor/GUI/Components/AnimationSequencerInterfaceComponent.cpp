#include "Editor/pch.hpp"
#include "AnimationSequencerInterfaceComponent.hpp"

#include <Scene/EntityComponentSystem/Components/ComponentBase.hpp>

#include <rttr/type.h>
#include <imgui_internal.h>

namespace Darius::Editor::Gui::Components
{
	AnimationSequence::AnimationSequence() :
		mFrameMin(0),
		mFrameMax(60),
		mExpanded(false)
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
		return (int)myItems.size();
	}

	int AnimationSequence::GetItemTypeCount() const
	{
		return (int)mProperties.size();
	}

	const char* AnimationSequence::GetItemTypeName(int typeIndex) const
	{
		if (typeIndex < 0 || typeIndex >= mProperties.size())
			return "";

		return mProperties.at(typeIndex).get_name().data();
	}

	const char* AnimationSequence::GetItemLabel(int index) const
	{
		if (index < 0 || index >= mProperties.size())
			return "";

		return mProperties.at(myItems[index].Type).get_name().data();
	}

	void AnimationSequence::Get(int index, int** start, int** end, int* type, unsigned int* color)
	{
		MySequenceItem& item = myItems[index];
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
		myItems.push_back(MySequenceItem{ type, 0, 10, true });
	};

	void AnimationSequence::Del(int index)
	{
		myItems.erase(myItems.begin() + index);
	}

	void AnimationSequence::Duplicate(int index)
	{
		myItems.push_back(myItems[index]);
	}

	size_t AnimationSequence::GetCustomHeight(int index)
	{
		return myItems[index].Expanded ? 300 : 0;
	}

	void AnimationSequence::DoubleClick(int index)
	{
		if (myItems[index].Expanded)
		{
			myItems[index].Expanded = false;
			return;
		}
		for (auto& item : myItems)
			item.Expanded = false;

		myItems[index].Expanded = !myItems[index].Expanded;
	}

	void AnimationSequence::CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
	{
		static const char* labels[] = { "Translation", "Rotation" , "Scale" };

		rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
		rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
		draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
		for (int i = 0; i < rampEdit.GetCurveCount(); i++)
		{
			ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
			ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
			draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
			if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
				rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
		}
		draw_list->PopClipRect();

		ImGui::SetCursorScreenPos(rc.Min);
		ImCurveEdit::Edit(rampEdit, rc.Max - rc.Min, 137 + index, &clippingRect);
	}

	void AnimationSequence::CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect)
	{
		rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
		rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
		draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < rampEdit.mPointCount[i]; j++)
			{
				float p = rampEdit.mPts[i][j].x;
				if (p < myItems[index].FrameStart || p > myItems[index].FrameEnd)
					continue;
				float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
				float x = ImLerp(rc.Min.x, rc.Max.x, r);
				draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
			}
		}
		draw_list->PopClipRect();
	}

	void AnimationSequence::SetReferenceComponent(ComponentBase* comp)
	{
		if (comp == mReferenceComponent)
			return;

		mProperties.clear();

		mReferenceComponent = comp;

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
			for (auto& prop : comp->get_type().get_properties())
			{
				if (prop.get_metadata("ANIMATE"))
					mProperties.push_back(prop);
			}

			std::sort(mProperties.begin(), mProperties.end(), [](rttr::property const& a, rttr::property const& b)
				{
					return a.get_name() < b.get_name();
				});

		}
	}

	char const* AnimationSequence::GetCollapseFmt() const
	{
		return mCollapsedDisplayName.c_str();
	}
}