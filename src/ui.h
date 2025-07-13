#pragma once

#include "raylib.h"

#include <functional>
#include <memory>
#include <span>

#include "fredlib.h"


namespace ui
{
    using ControlPtr = std::shared_ptr<struct ControlState>;
    using ControlHandlerPtr = std::shared_ptr<struct ControlHandler>;

    constexpr int kUiFontSize = 24;
    extern Font g_fredFont;


    ControlPtr CreateRootControl(int width, int height);
    const ControlPtr& GetRootControl();



    void LoadUIFont();

    void DrawUIText(const char* textUtf8, const Vector2& pos, const Color& col);
    void DrawUIText(const char* textUtf8, float x, float y, const Color& col);


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
    extern const ControlStyle kDefaultControlStyle;
    extern const ControlStyle kNullControlStyle;


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
    

    void AddChild(const ControlPtr& parentCtrlP, const ControlPtr& childCtrlP);

    inline Color GetBgColor(const ControlState& ctrl);
    inline Color GetFgColor(const ControlState& ctrl);


    Vector2 CalcMinDesiredSize(const ControlState& ctrl);
    void UpdateDesiredSize(ControlState& ctrl);
    void UpdateDesiredSizes(std::span<ControlPtr> children);


    struct Layout_HorizBox_Params
    {
        float Padding = 1;
        bool Shrink = true;     // if shrink is true, controls will be scaled down so they don't exceed the available space
        bool Expand = false;    // if expand is true, controls will be scaled up so they completely fill the available space
    };

    void Layout_HorizBox(const Rectangle& parent, std::span<ControlPtr> children, const Layout_HorizBox_Params& params);


    using Layout_VertBox_Params = Layout_HorizBox_Params;
    void Layout_VertBox(const Rectangle& parent, std::span<ControlPtr> children, const Layout_HorizBox_Params& params);
};


// inline func definitions ---------------------------------------------------------
namespace ui
{
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
}
