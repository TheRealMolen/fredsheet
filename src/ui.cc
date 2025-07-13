#include "ui.h"

#include "raymath.h"

#include <numeric>


namespace ui
{
    Font g_fredFont;


    void LoadUIFont()
    {
        g_fredFont = LoadFontEx("C:\\Windows\\Fonts\\calibril.ttf", kUiFontSize, nullptr, 0);
    }

    void DrawUIText(const char* textUtf8, const Vector2& pos, const Color& col)
    {
        DrawTextEx(g_fredFont, textUtf8, pos, kUiFontSize, 0.f, col);
    }
    void DrawUIText(const char* textUtf8, float x, float y, const Color& col)
    {
        DrawTextEx(g_fredFont, textUtf8, Vector2{ x, y }, kUiFontSize, 0.f, col);
    }


    Vector2 CalcMinDesiredSize(const ControlState& ctrl)
    {
        constexpr float kDefaultMinDim = 8.f;
        FRASSERT(ctrl.Style);

        Vector2 minContentSize(kDefaultMinDim, kDefaultMinDim);

        if (!ctrl.Text.empty())
        {
            const int fontSize = ctrl.Style->TextSize;
            const Vector2 textSize = MeasureTextEx(g_fredFont, ctrl.Text.c_str(), fontSize, 0.f);
            minContentSize = Vector2Max(minContentSize, textSize);
        }

        const Cardinals& pad = ctrl.Style->Padding;
        return Vector2Add(minContentSize, Vector2(pad.e + pad.w, pad.n + pad.s));
    }

    void UpdateDesiredSize(ControlState& ctrl)
    {
        ctrl.MinDesiredSize = CalcMinDesiredSize(ctrl);
        ctrl.DesiredSizeDirty = false;
    }

    void UpdateDesiredSizes(std::span<ControlPtr> children)
    {
        for (ControlPtr& ctrlP : children)
        {
            ControlState* ctrl = ctrlP.get();
            if (!ctrl)
                throw std::invalid_argument("null control in UpdateDesiredSizes");

            if (ctrl->DesiredSizeDirty)
                UpdateDesiredSize(*ctrl);
        }
    }


    void Layout_HorizBox(const Rectangle& parent, std::span<ControlPtr> children, const Layout_HorizBox_Params& params)
    {
        // refresh any out of date desired sizes
        UpdateDesiredSizes(children);

        const float minTotalChildWidth = std::accumulate(begin(children), end(children), 0.f, [](float acc, const auto& ctrlP) { return (acc + ctrlP->MinDesiredSize.x); });
        const int numChildren = isize(children);
        const float totalPadding = params.Padding * (numChildren + 1);

        const float availWidth = parent.width;
        
        const bool everythingFits = (minTotalChildWidth + totalPadding) <= availWidth;
        float scale = 1.f;
        if (everythingFits || !params.Shrink)
        {
            if (everythingFits && params.Expand)
            {
                scale = (availWidth - totalPadding) / minTotalChildWidth;
            }
        }
        else
        {
            scale = (availWidth - totalPadding) / minTotalChildWidth;
        }

        float trueX = parent.x + params.Padding;
        float intX = roundf(trueX);
        for (ControlPtr& ctrlP : children)
        {
            ControlState* ctrl = ctrlP.get();
            assert(ctrl);
            ctrl->Rect.y = parent.y;
            ctrl->Rect.height = parent.height;

            ctrl->Rect.x = intX;

            const float width = ctrl->MinDesiredSize.x * scale;
            trueX += width;
            intX = roundf(trueX);
            ctrl->Rect.width = intX - ctrl->Rect.x;

            trueX += params.Padding;
            intX = roundf(trueX);
        }
    }


    void Layout_VertBox(const Rectangle& parent, std::span<ControlPtr> children, const Layout_HorizBox_Params& params)
    {
        // refresh any out of date desired sizes
        UpdateDesiredSizes(children);

        const float minTotalChildHeight = std::accumulate(begin(children), end(children), 0.f, [](float acc, const auto& ctrlP) { return (acc + ctrlP->MinDesiredSize.y); });
        const int numChildren = isize(children);
        const float totalPadding = params.Padding * (numChildren + 1);

        const float availHeight = parent.height;
        
        const bool everythingFits = (minTotalChildHeight + totalPadding) <= availHeight;
        float scale = 1.f;
        if (everythingFits || !params.Shrink)
        {
            if (everythingFits && params.Expand)
            {
                scale = (availHeight - totalPadding) / minTotalChildHeight;
            }
        }
        else
        {
            scale = (availHeight - totalPadding) / minTotalChildHeight;
        }

        float trueY = parent.y + params.Padding;
        float intY = roundf(trueY);
        for (ControlPtr& ctrlP : children)
        {
            ControlState* ctrl = ctrlP.get();
            assert(ctrl);
            ctrl->Rect.x = parent.x;
            ctrl->Rect.width = parent.width;

            ctrl->Rect.y = intY;

            const float height = ctrl->MinDesiredSize.y * scale;
            trueY += height;
            intY = roundf(trueY);
            ctrl->Rect.height = intY - ctrl->Rect.y;

            trueY += params.Padding;
            intY = roundf(trueY);
        }
    }
};

