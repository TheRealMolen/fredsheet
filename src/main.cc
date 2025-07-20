#include "raylib.h"
#include "raymath.h"

#include <cstdlib>
#include <iostream>

#include "fredlib.h"
#include "ui.h"



void createControls(ui::ControlPtr& parent)
{
    FRASSERT(parent.get());
    
    ui::ControlHandlerPtr handler = make_shared<ui::ControlHandler>();
    handler->OnClick = [&parent](ui::ControlPtr& ctrlP)
    {
        ui::ControlState* ctrl = ctrlP.get();
        FRASSERT(ctrl);

        ctrl->Text = "CLICKED";
        ui::MarkDesiredSizeDirty(*ctrl);

        ui::RefreshControlLayout(parent);
    };
    
    ui::ControlPtr vb_toplevel = make_shared<ui::ControlState>("VB_TopLevel");
    vb_toplevel->Style = &ui::kNullControlStyle;
    vb_toplevel->LayoutParams = ui::Layout_Params { .Algo = ui::ELayoutAlgo::VertBox, .Padding = 10.f };
    ui::AddChild(parent, vb_toplevel);

    ui::AddChild(vb_toplevel, make_shared<ui::ControlState>("TL_Row1", "Row1"));
    
    ui::ControlPtr hb_cells = make_shared<ui::ControlState>("HB_Cells");
    hb_cells->LayoutParams = ui::Layout_Params { .Algo = ui::ELayoutAlgo::HorizBox, .Padding = 2.f };
    ui::AddChild(vb_toplevel, hb_cells);
    
    ui::AddChild(vb_toplevel, make_shared<ui::ControlState>("TL_Row3", "Row3"));
    ui::AddChild(vb_toplevel, make_shared<ui::ControlState>("TL_Row4", "Row4"));

    for (int i = 0; i < 12; ++i)
    {
        ui::ControlPtr ctrl = make_shared<ui::ControlState>("Cell");
        ctrl->Style = &ui::kDefaultControlStyle;
        ctrl->Handlers = handler;

        for (int n = 0; n <= i; ++n)
            ctrl->Text += 'X';

        ui::AddChild(hb_cells, ctrl);
    }

    ui::RefreshControlLayout(parent);
    
    std::cout << "post-refresh controls: -----------------------------------\n" << ui::DumpHierarchyToJson(parent) << "\n\n\n";
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
        ui::HandleInput();

        while (int key = GetKeyPressed())
        {
            if (key == 'd' || key == 'D')
            {
                std::cout << "hierarchy: -----------------------------------\n" << ui::DumpHierarchyToJson(ui::GetRootControl()) << "\n\n\n";
            }
        }

        if (ui::NeedsRepaint())
        {
            BeginDrawing();

            ui::Repaint();

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