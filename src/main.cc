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


static ui::ControlPtr gHoveredCtrl;
constexpr bool gOnlyRepaintWhenDirty = false;
static bool gNeedRepaint = true;



void createControls(ui::ControlPtr& parent)
{
    FRASSERT(parent.get());
    
    auto refreshChildrenLayout = [](ui::ControlPtr& parent)
    {
        ui::ControlState* parentCtrl = parent.get();
        if (!parentCtrl)
            return;

        ui::Layout_HorizBox(Rectangle { 0, 200, parentCtrl->Rect.width, 32 }, parentCtrl->Children, { .Padding = 2, .Expand = true });
        //ui::Layout_VertBox(Rectangle { 100, 0, 200, parentCtrl->Rect.height }, parentCtrl->Children, { .Padding=2, .Expand=false });
    };

    ui::ControlHandlerPtr handler = make_shared<ui::ControlHandler>();
    handler->OnClick = [&refreshChildrenLayout](const ui::ControlPtr& ctrlP)
    {
        ui::ControlState* ctrl = ctrlP.get();
        FRASSERT(ctrl);

        ctrl->Text = "CLICKED";
        ctrl->DesiredSizeDirty = 1;

        refreshChildrenLayout(ctrl->Parent);
    };

    for (int i = 0; i < 12; ++i)
    {
        ui::ControlPtr ctrl = make_shared<ui::ControlState>();
        ctrl->Style = &ui::kDefaultControlStyle;
        ctrl->Handlers = handler;

        for (int n = 0; n <= i; ++n)
            ctrl->Text += 'x';

        ui::AddChild(parent, ctrl);
    }

    refreshChildrenLayout(parent);
}



void updateHoveredCtrl(const Vector2& inputPos)
{
    if (gHoveredCtrl.get())
    {
        ui::ControlState* ctrl = gHoveredCtrl.get();
        if (IsPointInside(inputPos, ctrl->Rect))
            return;

        gHoveredCtrl.reset();
        ctrl->IsHovered = 0;
    }
    
    ui::ControlPtr rootCtrl = ui::GetRootControl();
    FRASSERT(rootCtrl.get());

    for (const ui::ControlPtr& ctrlP : rootCtrl->Children)
    {
        ui::ControlState* ctrl = ctrlP.get();
        if (!IsPointInside(inputPos, ctrl->Rect))
            continue;

        gHoveredCtrl = ctrlP;
        ctrl->IsHovered = 1;
        break;
    }

    gNeedRepaint = true;
}

void tryDispatchMouseClick()
{
    if (!gHoveredCtrl.get())
        return;

    ui::ControlHandlerPtr handlerP = gHoveredCtrl->Handlers;
    if (handlerP.get())
    {
        handlerP->OnClick(gHoveredCtrl);
    }
}


void RenderControl(const ui::ControlPtr& ctrlP)
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
            const Cardinals& pad = ctrl->Style->Padding;
            cursor.x += pad.e;
            cursor.y += pad.n;
        }

        const Color fgCol = ui::GetFgColor(*ctrl);
        DrawTextEx(ui::g_fredFont, ctrl->Text.c_str(), cursor, ui::kUiFontSize, 0.f, fgCol);
    }
}


void render()
{
    const ui::ControlState* rootCtrl = ui::GetRootControl().get();
    if (!rootCtrl)
    {
        ClearBackground(WHITE);
        return;
    }

    ClearBackground(rootCtrl->Style->NormalBg);

    for (const ui::ControlPtr& ctrlP : rootCtrl->Children)
    {
        RenderControl(ctrlP);
    }
}


int main(int argc, const char** argv)
{
    const int screenWidth = 1600;
    const int screenHeight = 1024;

    InitWindow(screenWidth, screenHeight, "fredsheet");
    ui::LoadUIFont();
    
    const ui::ControlStyle kRootControlStyle = { .NormalBg = DARKGRAY };
    ui::ControlPtr rootCtrl = ui::CreateRootControl(screenWidth, screenHeight);
    rootCtrl->Style = &kRootControlStyle;

    createControls(rootCtrl);


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