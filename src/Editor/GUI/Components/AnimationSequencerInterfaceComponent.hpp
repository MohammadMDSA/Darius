#pragma once

#include "PropertyAnimationCurve.hpp"

#include <Core/Containers/Vector.hpp>
#include <Core/Signal.hpp>

#include <ImSequencer.h>
#include <rttr/property.h>

#ifndef D_GUI_COMPONENT
#define D_GUI_COMPONENT Darius::Editor::Gui::Components
#endif

namespace Darius
{
	namespace Animation
	{
		class AnimationResource;
	}

	namespace Scene::ECS::Components
	{
		class ComponentBase;
	}
}

namespace Darius::Editor::Gui::Components
{
	struct AnimationSequence : public ImSequencer::SequenceInterface
	{
		using ComponentBase = Darius::Scene::ECS::Components::ComponentBase;

		struct SequenceItem
		{
			int										Type;
			std::shared_ptr<BasePropertyCurveEdit>	Curve;

			bool									Expanded;
		};

		AnimationSequence();

		virtual int                 GetFrameMin() const override;

		virtual int                 GetFrameMax() const override;

		virtual int                 GetItemCount() const override;

		virtual int                 GetItemTypeCount() const override;

		virtual const char*			GetItemTypeName(int typeIndex) const override;

		rttr::property const*		GetItemTypeProperty(int typeIndex) const;

		virtual const char*			GetItemLabel(int index) const override;

		virtual const char*			GetCollapseFmt() const override;

		virtual void                Get(int index, int** start, int** end, int* type, unsigned int* color) override;

		virtual void                Add(int type) override;

		virtual void                Del(int index) override;

		virtual void                Duplicate(int index) override;

		virtual size_t              GetCustomHeight(int index) override;

		virtual void                DoubleClick(int index) override;

		virtual void                CustomDraw(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect) override;

		virtual void                CustomDrawCompact(int index, ImDrawList* draw_list, const ImRect& rc, const ImRect& clippingRect) override;

		void						Initialize(ComponentBase* referenceComponent, D_ANIMATION::AnimationResource* animationRes, int& currentFrameRef);

		D_ANIMATION::Sequence const* GetSequence() const;

		bool mExpanded;

	private:

		void								InitializeSequenceItems();

		D_ANIMATION::AnimationResource*		mAnimationResource;
		ComponentBase*						mReferenceComponent;
		std::string							mComponentDisplayName;
		std::string							mCollapsedDisplayName;
		D_CONTAINERS::DVector<rttr::property> mAllProperties;
		std::vector<SequenceItem>			mPropertyCurves;
		int*								mCurrentFrame;
	};
}
