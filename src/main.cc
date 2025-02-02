#include "raylib.h"

#include <cstdint>
#include <cstdlib>
#include <vector>

using u8 = uint8_t;



using std::begin, std::end, std::size;
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
    const int kUiFontSize = 28;
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

    struct ControlState
    {
        Rectangle Rect { 0, 0, 100, 100 };
        const ControlStyle* style = &kDefaultControlStyle;

        u8 IsHovered : 1;
        u8 IsSelected : 1;
        u8 IsFocused : 1;
    };
};



static std::vector<ui::ControlState> gControls;
static int gHoveredCtrlIx = -1;



void initWindow()
{
    for (int i = 0; i < 1200; i += 80)
    {
        gControls.emplace_back(Rectangle { float(i), 200, 70, 40 });
    }
}

void updateHoveredCtrl(const Vector2& inputPos)
{
    if (gHoveredCtrlIx >= 0)
    {
        ui::ControlState& ctrl = gControls[gHoveredCtrlIx];
        if (IsPointInside(inputPos, ctrl.Rect))
            return;

        gHoveredCtrlIx = -1;
        ctrl.IsHovered = 0;
    }

    auto itCtrl = begin(gControls);
    for (int i = 0; i < isize(gControls); ++i, ++itCtrl)
    {
        ui::ControlState& ctrl = *itCtrl;
        if (!IsPointInside(inputPos, ctrl.Rect))
            continue;

        gHoveredCtrlIx = i;
        ctrl.IsHovered = 1;
        break;
    }
}


void render()
{
    ClearBackground(DARKGRAY);

    for (const ui::ControlState& ctrl : gControls)
    {
        Color bg = ctrl.style->NormalBg;
        if (ctrl.IsHovered)
            bg = ctrl.style->HoverBg;
        else if (ctrl.IsSelected)
            bg = ctrl.style->SelectedBg;


        DrawRectangle(ctrl.Rect.x, ctrl.Rect.y, ctrl.Rect.width, ctrl.Rect.height, bg);
    }

    
    ui::DrawUIText("oRrRange", 500, 150, ORANGE);

    Vector2 cursor = { 190, 200 };
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