#pragma once

#include "raylib.h"

#include <functional>
#include <memory>
#include <span>

#include "fredlib.h"


namespace ui
{
    constexpr int kDefaultFontSize = 24;


    using ControlPtr = std::shared_ptr<struct ControlState>;
    using ControlHandlerPtr = std::shared_ptr<struct ControlHandler>;
    using IconAtlasPtr = std::shared_ptr<struct IconAtlas>;

    ControlPtr CreateRootControl(int width, int height);
    const ControlPtr& GetRootControl();



    void LoadUIFont();

    void DrawUIText(const char* textUtf8, const Vector2& pos, const Color& col);
    void DrawUIText(const char* textUtf8, float x, float y, const Color& col);

    float MeasureTextHeight(float fontSize);

    IconAtlasPtr LoadIconAtlas(const char* filename, float iconSize);


    struct IconAtlas
    {
        Texture2D Texture;
        Vector2 IconSize;
    };
    struct IconRef
    {
        IconAtlasPtr Atlas;
        Vector2 Location;
    };


    struct ControlHandler
    {
        using EventHandler = std::function<void(const ControlPtr& ctrlP)>;
        using MouseEventHandler = std::function<void(ControlPtr& ctrlP, const Vector2& mousePos)>;

        MouseEventHandler OnClick;
        MouseEventHandler OnMenu;

        EventHandler Paint;    // only called if UseCustomPaint is true
    };


    struct ControlStyle
    {
        u8 DrawBg : 1 = 1;

        Color NormalBg = Color { 80, 80, 76, 255 };
        Color HoverBg = Color { 120, 120, 117, 255 };
        Color SelectedBg = Color { 100, 100, 97, 255 };;

        Color NormalFg = WHITE;
        Color HoverFg = WHITE;
        Color SelectedFg = YELLOW;

        int TextSize = kDefaultFontSize;

        Cardinals Padding { 8.f, 8.f, 4.f, 8.f };
    };
    extern const ControlStyle kDefaultControlStyle;
    extern const ControlStyle kNullControlStyle;
    extern const ControlStyle kBtnControlStyle;
    extern const ControlStyle kTightControlStyle;
    

    enum class ELayoutAlgo : u8
    {
        Fill,
        HorizBox,
        VertBox,
    };
    struct Layout_Params
    {
        ELayoutAlgo Algo = ELayoutAlgo::Fill;

        float Padding = 1;
        bool Shrink = true;     // if shrink is true, controls will be scaled down so they don't exceed the available space
        bool Expand = false;    // if expand is true, controls will be scaled up so they completely fill the available space
    };


    struct ControlState
    {
        const char* DbgName = nullptr;

        std::string Text;
        IconRef Icon;

        const ControlStyle* Style = &kNullControlStyle;

        ControlHandlerPtr Handlers;

        ControlPtr Parent;
        std::vector<ControlPtr> Children;

        Vector2 MinDesiredSize { 0.f, 0.f };
        u8 IsFixedSize : 1 = false;

        u8 UseCustomPaint : 1 = false;

        u8 IsHovered : 1 = false;
        u8 IsSelected : 1 = false;
        u8 IsFocused : 1 = false;

        u8 DesiredSizeDirty : 1 = true;

        Rectangle Rect { 0, 0, 100, 100 };

        Layout_Params LayoutParams;
    };
    
    
    // -- layout --------------------------------

    void AddChild(ControlPtr& parentCtrlP, ControlPtr childCtrlP);

    inline Color GetBgColor(const ControlState& ctrl);
    inline Color GetFgColor(const ControlState& ctrl);

    void MarkDesiredSizeDirty(ControlState& ctrl);
    Vector2 CalcMinDesiredOwnSize(const ControlState& ctrl);
    void UpdateDesiredSize(ControlState& ctrl);

    using Layout_HorizBox_Params = Layout_Params;
    void Layout_HorizBox(const Rectangle& parent, std::span<ControlPtr> children, const Layout_HorizBox_Params& params);

    using Layout_VertBox_Params = Layout_Params;
    void Layout_VertBox(const Rectangle& parent, std::span<ControlPtr> children, const Layout_HorizBox_Params& params);

    void RefreshControlLayout(ControlPtr& ctrl);


    // -- input ---------------------------------
    void HandleInput();
    
    // -- rendering ---------------------------------
    bool NeedsRepaint();
    void Repaint();

    // -- debug ---------------------------------
    std::string DumpHierarchyToJson(const ControlPtr& ctrl);

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
