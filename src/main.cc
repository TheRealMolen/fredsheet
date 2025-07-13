#include "raylib.h"
#include "raymath.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <numeric>
#include <span>
#include <stdexcept>
#include <sstream>
#include <vector>

#include "fredlib.h"
#include "ui.h"


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