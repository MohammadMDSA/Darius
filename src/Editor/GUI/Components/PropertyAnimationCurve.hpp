#pragma once

#include <Utils/Common.hpp>

#include <ImCurveEdit.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#ifndef D_GUI_COMPONENT
#define D_GUI_COMPONENT Darius::Editor::Gui::Components
#endif

namespace Darius::Editor::Gui::Components
{
    struct Vector3PropertyCurveEdit : public ImCurveEdit::Delegate
    {
        Vector3PropertyCurveEdit();

        INLINE virtual size_t GetCurveCount() override { return 3; }

        INLINE virtual bool IsVisible(size_t curveIndex) override
        {
            return mbVisible[curveIndex];
        }

        virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const override { return ImCurveEdit::CurveLinear; }

        virtual ImVec2& GetMax() override { return mMax; }
        virtual ImVec2& GetMin() override { return mMin; }

        INLINE virtual size_t GetPointCount(size_t curveIndex) override
        {
            return mPointCount[curveIndex];
        }

        INLINE virtual uint32_t GetCurveColor(size_t curveIndex) override
        {
            uint32_t cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
            return cols[curveIndex];
        }

        INLINE virtual ImVec2* GetPoints(size_t curveIndex) override
        {
            return mPts[curveIndex];
        }

        virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) override
        {
            mPts[curveIndex][pointIndex] = ImVec2(value.x, value.y);
            SortValues(curveIndex);
            for (size_t i = 0; i < GetPointCount(curveIndex); i++)
            {
                if (mPts[curveIndex][i].x == value.x)
                    return (int)i;
            }
            return pointIndex;
        }

        virtual void AddPoint(size_t curveIndex, ImVec2 value) override
        {
            if (mPointCount[curveIndex] >= 8)
                return;
            mPts[curveIndex][mPointCount[curveIndex]++] = value;
            SortValues(curveIndex);
        }

        virtual unsigned int GetBackgroundColor() override { return 0; }

        ImVec2 mPts[3][8];
        size_t mPointCount[3];
        bool mbVisible[3];
        ImVec2 mMin;
        ImVec2 mMax;
    private:
        void SortValues(size_t curveIndex)
        {
            auto b = std::begin(mPts[curveIndex]);
            auto e = std::begin(mPts[curveIndex]) + GetPointCount(curveIndex);
            std::sort(b, e, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });

        }
    };
}