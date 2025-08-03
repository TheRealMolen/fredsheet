
#include "raylib.h"
#include "raymath.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT
#include "test_framework.h"

#include "fredlib.h"
#include "ui.h"

#include "sheet.h"


// --------------------------------------------------------------------------------

void OnToolbarOpenClicked(ui::ControlPtr& ctrlP, const Vector2& mousePos)
{
    ChooseAndOpenSheet();
}

void OnToolbarBtnClicked(ui::ControlPtr& ctrlP, const Vector2& mousePos)
{
    ui::ControlState* ctrl = ctrlP.get();
    FRASSERT(ctrl);

    std::cout << ctrl->DbgName << " Clicked" << std::endl;
}


// returns the main client area control
ui::ControlPtr createTopLevelControls(ui::ControlPtr& parentP)
{
    FRASSERT(parentP.get());
    using std::make_shared;

    parentP->LayoutParams = ui::Layout_Params { .Algo = ui::ELayoutAlgo::VertBox, .Padding = 0, .Expand = false };
    
    ui::ControlPtr vb_toplevel = make_shared<ui::ControlState>("VB_TopLevel");
    vb_toplevel->Style = &ui::kNullControlStyle;
    vb_toplevel->LayoutParams = ui::Layout_Params { .Algo = ui::ELayoutAlgo::VertBox, .Padding = 10.f, .Expand = false };
    ui::AddChild(parentP, vb_toplevel);

    { // toolbar
        ui::IconAtlasPtr iconAtlas = ui::LoadIconAtlas("data\\icons.png", 32.f);

        ui::ControlPtr hb_toolbar = make_shared<ui::ControlState>("HB_ToolBar");
        hb_toolbar->LayoutParams = ui::Layout_Params { .Algo = ui::ELayoutAlgo::HorizBox, .Padding = 2.f };
        ui::AddChild(vb_toplevel, hb_toolbar);

        auto addToolbarBtn = [ & ](const char* dbgName, int locU, int locV, auto onclick)
        {
            ui::ControlPtr btn = make_shared<ui::ControlState>(dbgName);
            btn->Style = &ui::kBtnControlStyle;
            btn->Icon = { .Atlas = iconAtlas, .Location = { float(locU), float(locV) } };
            
            ui::ControlHandlerPtr toolbar_handler = make_shared<ui::ControlHandler>();
            toolbar_handler->OnClick = onclick;
            btn->Handlers = toolbar_handler;

            ui::AddChild(hb_toolbar, btn);
        };
        addToolbarBtn("Toolbar_New", 0, 0, OnToolbarBtnClicked);
        addToolbarBtn("Toolbar_Open", 1, 0, OnToolbarOpenClicked);
        addToolbarBtn("Toolbar_Save", 2, 0, OnToolbarBtnClicked);
    }

    ui::RefreshControlLayout(parentP);
    //std::cout << "post-refresh controls: -----------------------------------\n" << ui::DumpHierarchyToJson(parent) << "\n\n\n";

    ui::ControlPtr ctrl_main = make_shared<ui::ControlState>("MainEditor");
    ctrl_main->Style = &ui::kNullControlStyle;
    ctrl_main->IsFixedSize = true;
    ctrl_main->Rect = Rectangle {
        parentP->Rect.x, vb_toplevel->Rect.y + vb_toplevel->Rect.height,
        parentP->Rect.width, parentP->Rect.height,  // height is wrong, but ignored until we have a scrolling window -- by which time we'll hopefully be able to add VBox elements with FILL
    };
    ui::AddChild(parentP, ctrl_main);

    return ctrl_main;
}


int guardedMain()
{
    const int screenWidth = 1600;
    const int screenHeight = 1024;

    InitWindow(screenWidth, screenHeight, "fredsheet");
    ui::LoadUIFont();
    
    const ui::ControlStyle kRootControlStyle = { .NormalBg = DARKGRAY };
    ui::ControlPtr rootCtrl = ui::CreateRootControl(screenWidth, screenHeight);
    rootCtrl->Style = &kRootControlStyle;

    ui::ControlPtr editorCtrl = createTopLevelControls(rootCtrl);

    GridSheetPtr gridSheet = CreateNewSheet();
    GridSheetUIPtr gridSheetUI = InitSheetUI(gridSheet, editorCtrl);
    //std::cout << "post-grid controls: -----------------------------------\n" << ui::DumpHierarchyToJson(rootCtrl) << "\n\n\n";

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



int main(int argc, const char** argv)
{
    // first, run our unit tests
    // see https://github.com/doctest/doctest/blob/master/doc/markdown/main.md
    doctest::Context testContext;
    testContext.applyCommandLine(argc, argv);
    int testResult = testContext.run();
    if (testResult || testContext.shouldExit())
        return testResult;

    int returnCode = 0;
    try
    {
        returnCode = guardedMain();
    }
    catch(std::string& s)
    {
        std::cerr << "cataclysm: " << s << "\n";
        std::cerr.flush();

        OutputDebugStringA("Uncaught Exception: ");
        OutputDebugStringA(s.c_str());
        OutputDebugStringA("\n");

        returnCode = 1;
    }
    catch(std::exception& e)
    {
        std::cerr << "cataclysm: " << e.what() << "\n";
        std::cerr.flush();

        OutputDebugStringA("Uncaught Exception: ");
        OutputDebugStringA(e.what());
        OutputDebugStringA("\n");

        returnCode = 2;
    }
    catch(...)
    {
        std::cerr << "unknown cataclysm\n";
        std::cerr.flush();

        OutputDebugStringA("Uncaught Unknown Exception o_O\n");

        returnCode = 3;
    }

    return returnCode + testResult;
}