#pragma once

#include <Animation/AnimationCommon.hpp>
#include <Utils/Common.hpp>

#include <ImCurveEdit.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#ifndef D_GUI_COMPONENT
#define D_GUI_COMPONENT Darius::Editor::Gui::Components
#endif

namespace Darius::Editor::Gui::Components
{
    struct AnimationSequence;

    class BasePropertyCurveEdit : public ImCurveEdit::Delegate, public std::enable_shared_from_this<BasePropertyCurveEdit>
    {
    public:
        BasePropertyCurveEdit(AnimationSequence const* sequenceComp, rttr::property propertyRef, UINT framesPerSecond);

        virtual ImCurveEdit::CurveType  GetCurveType(size_t curveIndex) const override;

        virtual ImVec2&                 GetMax() override;
        virtual ImVec2&                 GetMin() override;

        INLINE virtual size_t           GetPointCount(size_t curveIndex) override
        {
            return GetTrack()->GetKeyframesCount();
        }

        INLINE float                    GetTime(UINT frameIndex) const { return (float)frameIndex / mFramesPerSecond; }
        INLINE UINT                     GetFrameIndex(float animationTime) const { return (UINT)(animationTime * mFramesPerSecond); }

        D_ANIMATION::Track*             GetTrack() const;

        virtual bool                    SetKeyframeValue(int frameIndex, rttr::property property, rttr::instance const& targetObject) = 0;

        UINT const&                     GetStartFrameIndex() const { return mFirstFrameIndex; }
        UINT const&                     GetLastFrameIndex() const { return mLastFrameIndexx; }

        INLINE rttr::property           GetPropertyRef() const { return mPropertyRef; }

    protected:

        void SortValues(size_t curveIndex);
        virtual void ReconstructPoints() = 0;

        ImVec2                                  mFrameMin;
        ImVec2                                  mFrameMax;
        UINT                                    mFirstFrameIndex;
        UINT                                    mLastFrameIndexx;

        rttr::property							mPropertyRef;
        AnimationSequence const* const          mSequenceComp;
        UINT const                              mFramesPerSecond;
    };

    template<typename ValueType, typename ElementType, UINT CurveCount>
    class GenericComposedPropertyCurveEdit : public BasePropertyCurveEdit
    {
    public:

        GenericComposedPropertyCurveEdit(AnimationSequence const* sequenceComp, rttr::property propertyRef, UINT framesPerSecond) :
            BasePropertyCurveEdit(sequenceComp, propertyRef, framesPerSecond)
        {
            ReconstructPoints();

            for (int i = 0; i < GetCurveCount(); i++)
            {
                mVisible[i] = true;
            }
        }

        INLINE virtual size_t GetCurveCount() override { return CurveCount; }

        INLINE virtual bool IsVisible(size_t curveIndex) override
        {
            return mVisible[curveIndex];
        }

        INLINE virtual void SetVisible(size_t curveIndex, bool value) override
        {
            mVisible[curveIndex] = value;
        }

        virtual char const* GetCurveName(size_t curveIndex) const override
        {
            switch (curveIndex)
            {
            case 0:
                return "X";
            case 1:
                return "Y";
            case 2:
                return "Z";
            default:
                return "";
            }
        }

        int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) override
        {

            D_ANIMATION::Keyframe& keyframe = GetTrack()->GetKeyframes()[pointIndex];

            ElementType* kfValue = (ElementType*)&keyframe.GetValue<ValueType>();
            kfValue[curveIndex] = static_cast<ElementType>(value.y);
            keyframe.Time = GetTime((UINT)value.x);

            SortValues(curveIndex);

            for (int i = 0; i < GetPointCount(0); i++)
            {
                if (mPts[0][i].x == value.x)
                    return i;
            }

            return pointIndex;
        }

        virtual void ReconstructPoints() override
        {
            for (int i = 0; i < GetCurveCount(); i++)
            {
                mPts[i].clear();
                mPts[i].resize(GetTrack()->GetKeyframesCount());
            }

            mFrameMax = ImVec2(0.f, 0.f);
            mFrameMin = ImVec2(0.f, 0.f);

            auto& keyframes = GetTrack()->GetKeyframes();
            if (keyframes.size() > 0)
            {
                mFrameMax.x = (float)GetFrameIndex(keyframes[keyframes.size() - 1].Time);
                mFrameMax.y = -FLT_MAX;
                mFrameMin.x = (float)GetFrameIndex(keyframes[0].Time);
                mFrameMin.y = FLT_MAX;
            }

            int index = 0;
            for (auto& kf : keyframes)
            {
                auto& value = kf.GetValue<ValueType>();
                ElementType* rawValue = (ElementType*)&value;

                UINT frameIndex = GetFrameIndex(kf.Time);

                for (int i = 0; i < GetCurveCount(); i++)
                {
                    float v = (float)rawValue[i];
                    mPts[i][index] = ImVec2((float)frameIndex, v);

                    // Deciding the max value
                    if (mFrameMin.y > v)
                        mFrameMin.y = v;
                    if (mFrameMax.y < v)
                        mFrameMax.y = v;
                }

                index++;
            }

            mFirstFrameIndex = (UINT)mFrameMin.x;
            mLastFrameIndexx = (UINT)mFrameMax.x;
        }

        void AddPoint(size_t curveIndex, ImVec2 value) override
        {

            D_ANIMATION::Keyframe* kf = GetTrack()->FindOrCreateKeyframeByTime((float)GetTime((UINT)value.x));
            auto& refValue = kf->GetValue<ValueType>();

            ElementType* rawValue = reinterpret_cast<ElementType*>(&refValue);

            rawValue[curveIndex] = static_cast<ElementType>(value.y);
            for (int i = 0; i < GetCurveCount(); i++)
            {
                if (i == curveIndex)
                    rawValue[i] = static_cast<ElementType>(value.y);
                else
                    rawValue[i] = static_cast<ElementType>(0);
            }

            SortValues(curveIndex);
        }

        INLINE virtual bool SetKeyframeValue(int frameIndex, rttr::property property, rttr::instance const& targetObject) override
        {
            if (!D_VERIFY(targetObject))
                return false;

            if (!D_VERIFY(property.is_valid()))
                return false;

            auto targetValueVar = property.get_value(targetObject);
            if (!D_VERIFY(targetValueVar.is_valid()))
                return false;

            if (!D_VERIFY(targetValueVar.is_type<ValueType>()))
                return false;

            ValueType value = targetValueVar.get_value<ValueType>();

            float keyframeTime = GetTime(frameIndex);

            bool created;
            D_ANIMATION::Keyframe* kf = GetTrack()->FindOrCreateKeyframeByTime(keyframeTime, &created);

            ValueType& destination = kf->GetValue<ValueType>();

            // Neither Keyframe created nor value changed
            if (!created && destination == value)
                return false;

            destination = value;

            SortValues(-1 /* Don't care param */);

            return true;
        }

        INLINE virtual uint32_t GetCurveColor(size_t curveIndex) override
        {
            D_STATIC_ASSERT(CurveCount <= 4);
            static const uint32_t cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF00FF, 0xFFFFFFFF };
            return cols[curveIndex];
        }

        INLINE virtual ImVec2* GetPoints(size_t curveIndex) override
        {
            return mPts[curveIndex].data();
        }

        virtual unsigned int GetBackgroundColor() override { return 0; }


    private:

        D_CONTAINERS::DVector<ImVec2>           mPts[CurveCount];
        bool                                    mVisible[CurveCount];

    };

    class Vector2PropertyCurveEdit : public GenericComposedPropertyCurveEdit<D_MATH::Vector2, D_MATH::Vector2::ElementType, 2>
    {
    public:
        Vector2PropertyCurveEdit(AnimationSequence const* sequenceComp, rttr::property propertyRef, UINT framesPerSecond) :
            GenericComposedPropertyCurveEdit(sequenceComp, propertyRef, framesPerSecond)
        {}
    };

    class Vector3PropertyCurveEdit : public GenericComposedPropertyCurveEdit<D_MATH::Vector3, D_MATH::Vector3::ElementType, 3>
    {
    public:
        Vector3PropertyCurveEdit(AnimationSequence const* sequenceComp, rttr::property propertyRef, UINT framesPerSecond) :
            GenericComposedPropertyCurveEdit(sequenceComp, propertyRef, framesPerSecond)
        {}
    };

    class Vector4PropertyCurveEdit : public GenericComposedPropertyCurveEdit<D_MATH::Vector4, D_MATH::Vector3::ElementType, 4>
    {
    public:
        Vector4PropertyCurveEdit(AnimationSequence const* sequenceComp, rttr::property propertyRef, UINT framesPerSecond) :
            GenericComposedPropertyCurveEdit(sequenceComp, propertyRef, framesPerSecond)
        {}
    };

    class ColorPropertyCurveEdit : public GenericComposedPropertyCurveEdit<D_MATH::Color, float, 4>
    {
    public:
        ColorPropertyCurveEdit(AnimationSequence const* sequenceComp, rttr::property propertyRef, UINT framesPerSecond) :
            GenericComposedPropertyCurveEdit(sequenceComp, propertyRef, framesPerSecond)
        {}
    };

    class QuaternionPropertyCurveEdit : public GenericComposedPropertyCurveEdit<D_MATH::Quaternion, float, 4>
    {
    public:
        QuaternionPropertyCurveEdit(AnimationSequence const* sequenceComp, rttr::property propertyRef, UINT framesPerSecond) :
            GenericComposedPropertyCurveEdit(sequenceComp, propertyRef, framesPerSecond)
        {}

        virtual bool SetKeyframeValue(int frameIndex, rttr::property property, rttr::instance const& targetObject) override;

    };
}