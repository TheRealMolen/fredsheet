
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


// --------------------------------------------------------------------------------

struct GridSheet
{
    using StringList = std::vector<std::string>;
    using StringGrid = std::vector<StringList>;

    StringGrid RawCells;    // the backing data - what's serialised to disk

    static constexpr int kDefaultNumCols = 20;
    static constexpr int kDefaultRowWidth = 80;

    std::vector<int> ColWidths { kDefaultNumCols, kDefaultRowWidth };

    std::weak_ptr<struct GridSheetUI> UI;
};
using GridSheetPtr = std::shared_ptr<GridSheet>;

// --------------------------------------------------------------------------------

struct GridSheetUI
{
    using ControlList = std::vector<ui::ControlPtr>;
    using ControlGrid = std::vector<ControlList>;

    std::shared_ptr<GridSheet> Sheet;

    ui::ControlPtr ParentCtrl;

    ControlList ColHeaderControls;
    ControlList RowHeaderControls;

    ControlGrid CellControls;
};
using GridSheetUIPtr = std::shared_ptr<GridSheetUI>;

// --------------------------------------------------------------------------------

GridSheetPtr CreateNewSheet()
{
    GridSheetPtr sheet = make_shared<GridSheet>();

    // TODO: set up default rows, columns, etc

    return sheet;
}

// --------------------------------------------------------------------------------



// --------------------------------------------------------------------------------

void AddRows(GridSheetUI& ui, const GridSheet& sheet, int numRowsToAdd)
{
    FRASSERT(ui.ParentCtrl.get());
    // TODO: add new row controls
    // TODO: add new cell controls
}

void AddRows(GridSheet& sheet, int numRowsToAdd)
{
    const int numCols = isize(sheet.ColWidths);

    // add raw cells
    sheet.RawCells.reserve(isize(sheet.RawCells) + numRowsToAdd);
    for (int r = 0; r < numRowsToAdd; ++r)
    {
        sheet.RawCells.emplace_back(numCols);
    }

    if (auto uiP = sheet.UI.lock())
    {
        AddRows(*uiP.get(), sheet, numRowsToAdd);
    }
}



// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

void OnToolbarBtnClicked(ui::ControlPtr& ctrlP)
{
    ui::ControlState* ctrl = ctrlP.get();
    FRASSERT(ctrl);

    std::cout << ctrl->DbgName << " Clicked" << std::endl;
}


void createControls(ui::ControlPtr& parent)
{
    FRASSERT(parent.get());

    ui::IconAtlasPtr iconAtlas = ui::LoadIconAtlas("data\\icons.png", 32.f);
    
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


    ui::ControlPtr hb_toolbar = make_shared<ui::ControlState>("HB_ToolBar");
    hb_toolbar->LayoutParams = ui::Layout_Params { .Algo = ui::ELayoutAlgo::HorizBox, .Padding = 2.f };
    ui::AddChild(vb_toplevel, hb_toolbar);

    ui::ControlHandlerPtr toolbar_handler = make_shared<ui::ControlHandler>();
    toolbar_handler->OnClick = OnToolbarBtnClicked;
    auto addToolbarBtn = [ & ](const char* dbgName, int locU, int locV)
    {
        ui::ControlPtr btn = make_shared<ui::ControlState>(dbgName);
        btn->Style = &ui::kBtnControlStyle;
        btn->Icon = { .Atlas = iconAtlas, .Location = { float(locU), float(locV) } };
        btn->Handlers = toolbar_handler;
        ui::AddChild(hb_toolbar, btn);
    };
    addToolbarBtn("Toolbar_New", 0, 0);
    addToolbarBtn("Toolbar_Open", 1, 0);
    addToolbarBtn("Toolbar_Save", 2, 0);

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
    
    //std::cout << "post-refresh controls: -----------------------------------\n" << ui::DumpHierarchyToJson(parent) << "\n\n\n";
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

    return returnCode;
}