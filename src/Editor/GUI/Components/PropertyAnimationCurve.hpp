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
    struct Vector3PropertyCurveEdit : public ImCurveEdit::Delegate, public std::enable_shared_from_this<Vector3PropertyCurveEdit>
    {
        Vector3PropertyCurveEdit(D_ANIMATION::Track* track, UINT framesPerSecond);

        INLINE virtual size_t GetCurveCount() override { return 3; }

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

        virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const override;

        virtual ImVec2& GetMax() override { return mMax; }
        virtual ImVec2& GetMin() override { return mMin; }

        INLINE virtual size_t GetPointCount(size_t curveIndex) override
        {
            return mTrack->GetKeyframesCount();
        }

        INLINE virtual uint32_t GetCurveColor(size_t curveIndex) override
        {
            static const uint32_t cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
            return cols[curveIndex];
        }

        INLINE virtual ImVec2* GetPoints(size_t curveIndex) override
        {
            return mPts[curveIndex].data();
        }

        virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) override;

        virtual void AddPoint(size_t curveIndex, ImVec2 value) override;

        virtual unsigned int GetBackgroundColor() override { return 0; }

        INLINE float GetTime(UINT frameIndex) const { return (float)frameIndex / mFramesPerSecond; }
        INLINE UINT GetFrameIndex(float animationTime) const { return (UINT)(animationTime * mFramesPerSecond); }


    private:


        void SortValues(size_t curveIndex);

        void ReconstructPoints();

        D_CONTAINERS::DVector<ImVec2>           mPts[3];
        ImVec2                                  mMin;
        ImVec2                                  mMax;
        bool                                    mVisible[3];

        D_ANIMATION::Track* const               mTrack;
        UINT const                              mFramesPerSecond;
    };
}