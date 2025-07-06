#include "raylib.h"
#include "raymath.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <memory>
#include <numeric>
#include <span>
#include <stdexcept>
#include <sstream>
#include <vector>

using u8 = uint8_t;
using u32 = uint32_t;


#define FRASSERT    assert



using std::begin, std::end, std::size;
using std::weak_ptr, std::shared_ptr, std::make_shared;

inline int isize(const auto& cont)
{
    return int(cont.size());
}


struct Cardinals
{
    float n;
    float e;
    float s;
    float w;

    Cardinals(float all) : n(all), e(all), s(all), w(all) { /**/ }
    Cardinals(float ns, float ew) : n(ns), e(ew), s(ns), w(ew) { /**/ }
    Cardinals(float n, float e, float s, float w) : n(n), e(e), s(s), w(w) { /**/ }
};



inline bool IsPointInside(const Vector2& pt, const Rectangle& rect)
{
    if (pt.x < rect.x || pt.y < rect.y)
        return false;
    if (pt.x >= (rect.x + rect.width) || (pt.y >= rect.y + rect.height))
        return false;

    return true;
}


namespace ui
{
    const int kUiFontSize = 24;
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


    using ControlPtr = std::shared_ptr<struct ControlState>;
    using ControlHandlerPtr = std::shared_ptr<struct ControlHandler>;

    struct ControlHandler
    {
        using EventHandler = std::function<void(const ControlPtr& ctrlP)>;

        EventHandler OnClick;
        EventHandler OnMenu;
    };


    struct ControlStyle
    {
        u8 DrawBg : 1 = 1;

        Color NormalBg = Color { 100, 100, 98, 255 };
        Color HoverBg = Color { 120, 120, 117, 255 };
        Color SelectedBg = GRAY;

        Color NormalFg = WHITE;
        Color HoverFg = WHITE;
        Color SelectedFg = YELLOW;

        int TextSize = kUiFontSize;

        Cardinals Padding { 8.f };
    };
    const ControlStyle kDefaultControlStyle;
    const ControlStyle kNullControlStyle = { .DrawBg = 0 };


    struct ControlState
    {
        std::string Text;
        const ControlStyle* Style = &kNullControlStyle;

        ControlHandlerPtr Handlers;

        ControlPtr Parent;
        std::vector<ControlPtr> Children;

        Vector2 MinDesiredSize { 0.f, 0.f };

        u8 IsHovered : 1 = false;
        u8 IsSelected : 1 = false;
        u8 IsFocused : 1 = false;

        u8 DesiredSizeDirty : 1 = true;

        Rectangle Rect { 0, 0, 100, 100 };
    };
    
    inline Color GetBgColor(const ControlState& ctrl)
    {
        FRASSERT(ctrl.Style);

        if (ctrl.IsHovered)
            return ctrl.Style->HoverBg;

        if (ctrl.IsSelected)
            return ctrl.Style->SelectedBg;

        return ctrl.Style->NormalBg;
    }
    inline Color GetFgColor(const ControlState& ctrl)
    {
        FRASSERT(ctrl.Style);

        if (ctrl.IsHovered)
            return ctrl.Style->HoverFg;

        if (ctrl.IsSelected)
            return ctrl.Style->SelectedFg;

        return ctrl.Style->NormalFg;
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


    struct Layout_HorizBox_Params
    {
        float Padding = 1;
        bool Shrink = true;     // if shrink is true, controls will be scaled down so they don't exceed the available space
        bool Expand = false;    // if expand is true, controls will be scaled up so they completely fill the available space
    };

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


    using Layout_VertBox_Params = Layout_HorizBox_Params;

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



static std::vector<ui::ControlPtr> gControls;
static int gHoveredCtrlIx = -1;
constexpr bool gOnlyRepaintWhenDirty = false;
static bool gNeedRepaint = true;



void initWindow()
{
    ui::ControlHandlerPtr handler = make_shared<ui::ControlHandler>();
    handler->OnClick = [](const ui::ControlPtr& ctrlP)
    {
        ui::ControlState* ctrl = ctrlP.get();
        assert(ctrl);

        ctrl->Text = "CLICKED";
        ctrl->DesiredSizeDirty = 1;
        
        ui::Layout_HorizBox(Rectangle { 0, 200, 1600, 32 }, gControls, { .Padding=2, .Expand=true });
    };

    for (int i = 0; i < 12; ++i)
    {
        ui::ControlPtr ctrl = make_shared<ui::ControlState>();
        ctrl->Style = &ui::kDefaultControlStyle;
        ctrl->Handlers = handler;

        for (int n = 0; n <= i; ++n)
            ctrl->Text += 'x';

        gControls.push_back(ctrl);
    }
    ui::Layout_HorizBox(Rectangle { 0, 200, 1600, 32 }, gControls, { .Padding=2, .Expand=true });
    //ui::Layout_VertBox(Rectangle { 100, 0, 200, 1024 }, gControls, { .Padding=2, .Expand=false });
}



void updateHoveredCtrl(const Vector2& inputPos)
{
    if (gHoveredCtrlIx >= 0)
    {
        ui::ControlState* ctrl = gControls[gHoveredCtrlIx].get();
        assert(ctrl);
        if (IsPointInside(inputPos, ctrl->Rect))
            return;

        gHoveredCtrlIx = -1;
        ctrl->IsHovered = 0;
    }

    auto itCtrl = begin(gControls);
    for (int i = 0; i < isize(gControls); ++i, ++itCtrl)
    {
        ui::ControlState* ctrl = itCtrl->get();
        if (!IsPointInside(inputPos, ctrl->Rect))
            continue;

        gHoveredCtrlIx = i;
        ctrl->IsHovered = 1;
        break;
    }

    gNeedRepaint = true;
}

void tryDispatchMouseClick()
{
    if (gHoveredCtrlIx < 0)
        return;
    
    ui::ControlPtr ctrlP = gControls[gHoveredCtrlIx];
    assert(ctrlP.get());

    ui::ControlHandlerPtr handlerP = ctrlP->Handlers;
    if (handlerP.get())
    {
        handlerP->OnClick(ctrlP);
    }
}


void render()
{
    ClearBackground(DARKGRAY);

    for (const ui::ControlPtr& ctrlP : gControls)
    {
        const ui::ControlState* ctrl = ctrlP.get();
        if (!ctrl)
            throw std::invalid_argument("null control in render");

        if (ctrl->Style && ctrl->Style->DrawBg)
        {
            const Color bgCol = ui::GetBgColor(*ctrl);
            DrawRectangle(ctrl->Rect.x, ctrl->Rect.y, ctrl->Rect.width, ctrl->Rect.height, bgCol);
        }

        if (!ctrl->Text.empty())
        {
            Vector2 cursor = { ctrl->Rect.x, ctrl->Rect.y };
            if (ctrl->Style)
            {
                const Cardinals pad = ctrl->Style->Padding;
                cursor.x += pad.e;
                cursor.y += pad.n;
            }

            const Color fgCol = ui::GetFgColor(*ctrl);
            DrawTextEx(ui::g_fredFont, ctrl->Text.c_str(), cursor, ui::kUiFontSize, 0.f, fgCol);
        }
    }
}


int main(int argc, const char** argv)
{
    const int screenWidth = 1600;
    const int screenHeight = 1024;

    InitWindow(screenWidth, screenHeight, "fredsheet");
    ui::LoadUIFont();

    initWindow();


    SetTargetFPS(30);

    while (!WindowShouldClose())
    {
        updateHoveredCtrl(GetMousePosition());

        if (IsMouseButtonReleased(0))
        {
            tryDispatchMouseClick();
        }

        if (!gOnlyRepaintWhenDirty || gNeedRepaint)
        {
            gNeedRepaint = false;

            BeginDrawing();

            render();

            EndDrawing();
        }
        else
        {
            WaitTime(0.05);
            PollInputEvents();
        }
    }

    CloseWindow();

    return 0;
}