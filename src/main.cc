#include "raylib.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <numeric>
#include <span>
#include <sstream>
#include <vector>

using u8 = uint8_t;



using std::begin, std::end, std::size;
using std::weak_ptr, std::shared_ptr, std::make_shared;

inline int isize(const auto& cont)
{
    return int(cont.size());
}



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



    struct ControlStyle
    {
        Color NormalBg = Color { 100, 100, 98, 255 };
        Color HoverBg = LIGHTGRAY;
        Color SelectedBg = GRAY;
    };
    const ControlStyle kDefaultControlStyle;

    using ControlPtr = std::shared_ptr<struct ControlState>;
    struct ControlState
    {
        Rectangle Rect { 0, 0, 100, 100 };
        std::string Text;
        const ControlStyle* Style = &kDefaultControlStyle;

        u8 IsHovered : 1;
        u8 IsSelected : 1;
        u8 IsFocused : 1;
    };


    struct Layout_HorizBox_Params
    {
        int Padding = 1;
        bool Shrink = true;     // if shrink is true, controls will be scaled down so they don't exceed the available space
        bool Expand = false;    // if expand is true, controls will be scaled up so they completely fill the available space
    };

    void Layout_HorizBox(const Rectangle& parent, std::span<ControlPtr> children, const Layout_HorizBox_Params& params)
    {
        const float minTotalChildWidth = std::accumulate(begin(children), end(children), 0.f, [](float acc, const auto& ctrlP) { return (acc + ctrlP->Rect.width); });
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
            ctrl->Rect.y = parent.y;
            ctrl->Rect.height = parent.height;

            ctrl->Rect.x = intX;

            const float width = ctrl->Rect.width * scale;
            trueX += width;
            intX = roundf(trueX);
            ctrl->Rect.width = intX - ctrl->Rect.x;

            trueX += params.Padding;
            intX = roundf(trueX);
        }
    }
};



static std::vector<ui::ControlPtr> gControls;
static int gHoveredCtrlIx = -1;



void initWindow()
{
    for (int i = 0; i < 16; ++i)
    {
        ui::ControlPtr ctrl = make_shared<ui::ControlState>();

        std::ostringstream os;
        os << "col " << i;
        ctrl->Text = os.str();

        gControls.push_back(ctrl);
    }
    ui::Layout_HorizBox(Rectangle { 0, 200, 1280, 32 }, gControls, { .Padding=2, .Expand=true });
}

void updateHoveredCtrl(const Vector2& inputPos)
{
    if (gHoveredCtrlIx >= 0)
    {
        ui::ControlState* ctrl = gControls[gHoveredCtrlIx].get();
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
}


void render()
{
    ClearBackground(DARKGRAY);

    for (const ui::ControlPtr& ctrlP : gControls)
    {
        const ui::ControlState* ctrl = ctrlP.get();

        Color bg = ctrl->Style->NormalBg;
        if (ctrl->IsHovered)
            bg = ctrl->Style->HoverBg;
        else if (ctrl->IsSelected)
            bg = ctrl->Style->SelectedBg;


        DrawRectangle(ctrl->Rect.x, ctrl->Rect.y, ctrl->Rect.width, ctrl->Rect.height, bg);

        if (!ctrl->Text.empty())
        {
            Vector2 cursor = { ctrl->Rect.x, ctrl->Rect.y };
            DrawTextEx(ui::g_fredFont, ctrl->Text.c_str(), cursor, ui::kUiFontSize, 0.f, WHITE);
        }
    }

    
    ui::DrawUIText("oRrRange", 500, 150, ORANGE);

    Vector2 cursor = { 190, 220 };
    for (int i = 0; i < 10; ++i)
    {
        DrawTextEx(ui::g_fredFont, "Banana", cursor, ui::kUiFontSize, 0.f, YELLOW);

        cursor.x += 80;
        cursor.y += ui::kUiFontSize;
    }
}


int main(int argc, const char** argv)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "fredsheet");
    ui::LoadUIFont();

    initWindow();


    SetTargetFPS(30);

    while (!WindowShouldClose())
    {
        updateHoveredCtrl(GetMousePosition());


        BeginDrawing();

        render();

        EndDrawing();
    }

    CloseWindow();

    return 0;
}