#include "ui.h"

#include "geom.h"

#include "raymath.h"

#include <cmath>
#include <numeric>
#include <optional>
#include <span>


namespace ui
{
    // functional switches
    constexpr bool gOnlyRepaintWhenDirty = false;
    constexpr bool gRenderDebugColouredControls = false;


    int gUiFontSize = kDefaultFontSize;
    std::optional<Font> gCoreFont;

    const ControlStyle kDefaultControlStyle;
    const ControlStyle kNullControlStyle = { .DrawBg = 0 };
    const ControlStyle kBtnControlStyle = { .DrawBg = 0, .NormalFg = Color {220, 220, 220, 255}, .HoverFg = YELLOW, .Padding { 2.f } };
    const ControlStyle kTightControlStyle = { .Padding { 1.f } };
    
    ControlPtr gRootControl;

    static ui::ControlPtr gHoveredCtrl;

    static bool gNeedRepaint = true;



    void LoadUIFont()
    {
        gCoreFont = LoadFontEx("data\\fonts\\montserrat\\Montserrat-Light.ttf", gUiFontSize, nullptr, 0);
    }

    void DrawUIText(const char* textUtf8, const Vector2& pos, const Color& col)
    {
        if (!gCoreFont.has_value())
            throw std::invalid_argument("no core UI font loaded");

        DrawTextEx(*gCoreFont, textUtf8, pos, gUiFontSize, 0.f, col);
    }
    void DrawUIText(const char* textUtf8, float x, float y, const Color& col)
    {
        if (!gCoreFont.has_value())
            throw std::invalid_argument("no core UI font loaded");

        DrawTextEx(*gCoreFont, textUtf8, Vector2{ x, y }, gUiFontSize, 0.f, col);
    }

    float MeasureTextHeight(float fontSize)
    {
        if (!gCoreFont.has_value())
            throw std::invalid_argument("no core UI font loaded");
        
        const Vector2 textSize = MeasureTextEx(*gCoreFont, "Mp", fontSize, 0.f);
        return textSize.y;
    }


    
    IconAtlasPtr LoadIconAtlas(const char* filename, float iconSize)
    {
        IconAtlasPtr atlasP = std::make_shared<IconAtlas>();
        IconAtlas* atlas = atlasP.get();

        atlas->Texture = LoadTexture(filename);
        if (!atlas->Texture.id)
            return {};

        atlas->IconSize = Vector2(iconSize, iconSize);

        return atlasP;
    }


    ControlPtr CreateRootControl(int width, int height)
    {
        FRASSERT(!gRootControl.get());

        ui::ControlPtr ctrl = std::make_shared<ui::ControlState>("ROOT");
        ctrl->Rect = { 0.f, 0.f, float(width), float(height) };
        ctrl->Style = &kNullControlStyle;

        gRootControl = ctrl;

        return ctrl;
    }

    const ControlPtr& GetRootControl()
    {
        return gRootControl;
    }


    void AddChild(ControlPtr& parentCtrlP, ControlPtr childCtrlP)
    {
        ControlState* parent = parentCtrlP.get();
        ControlState* child = childCtrlP.get();
        FRASSERT(parent);
        FRASSERT(child);

        // TODO
        FRASSERT(!child->Parent.get());

        child->Parent = parentCtrlP;
        parent->Children.push_back(childCtrlP);

        MarkDesiredSizeDirty(*parent);
    }

    
    // -------------------------------------------------------------------------------------------------
    struct ControlMeasurements
    {
        float Width = 0.f;
        float Height = 0.f;
        
        float PadWidth = 0.f;
        float PadHeight = 0.f;
    };


    // helpers for clarity
    inline float SumDesiredWidths(std::span<const ControlPtr> controls)
    {
        float sum = 0.f;
        for (const ControlPtr& ctrlP : controls)
            sum += ctrlP->MinDesiredSize.x;
        return sum;
    }
    inline float SumDesiredHeights(std::span<const ControlPtr> controls)
    {
        float sum = 0.f;
        for (const ControlPtr& ctrlP : controls)
            sum += ctrlP->MinDesiredSize.y;
        return sum;
    }
    inline float MaxDesiredWidth(std::span<const ControlPtr> controls)
    {
        float max = 0.f;
        for (const ControlPtr& ctrlP : controls)
            max = std::max(ctrlP->MinDesiredSize.x, max);
        return max;
    }
    inline float MaxDesiredHeight(std::span<const ControlPtr> controls)
    {
        float max = 0.f;
        for (const ControlPtr& ctrlP : controls)
            max = std::max(ctrlP->MinDesiredSize.y, max);
        return max;
    }


    ControlMeasurements Measure_HorizBox(const Rectangle& parentRect, std::span<const ControlPtr> children, const Layout_HorizBox_Params& params)
    {
        const float minTotalChildWidth = SumDesiredWidths(children);
        const float maxChildHeight = MaxDesiredHeight(children);
        const int numChildren = isize(children);

        const float xPadding = params.Padding * (numChildren + 1);
        const float yPadding = params.Padding * 2;

        return {
            .Width = minTotalChildWidth + xPadding,
            .Height = maxChildHeight + yPadding,
            .PadWidth = xPadding,
            .PadHeight = yPadding,
        };
    }

    void Layout_HorizBox(const Rectangle& parentRect, std::span<ControlPtr> children, const Layout_HorizBox_Params& params)
    {
        const ControlMeasurements measurements = Measure_HorizBox(parentRect, children, params);
        const float availWidth = parentRect.width;
        
        const bool everythingFits = measurements.Width <= availWidth;
        const bool scaleToFill = (!everythingFits && params.Shrink) || (everythingFits && params.Expand);

        float scale = 1.f;
        if (scaleToFill)
            scale = (availWidth - measurements.PadWidth) / (measurements.Width - measurements.PadWidth);

        float trueX = parentRect.x + params.Padding;
        float intX = roundf(trueX);
        for (ControlPtr& ctrlP : children)
        {
            ControlState* ctrl = ctrlP.get();
            assert(ctrl);
            ctrl->Rect.y = parentRect.y + params.Padding;
            ctrl->Rect.height = parentRect.height;

            ctrl->Rect.x = intX;

            const float width = ctrl->MinDesiredSize.x * scale;
            trueX += width;
            intX = roundf(trueX);
            ctrl->Rect.width = intX - ctrl->Rect.x;

            trueX += params.Padding;
            intX = roundf(trueX);
        }
    }

    

    ControlMeasurements Measure_VertBox(const Rectangle& parentRect, std::span<const ControlPtr> children, const Layout_VertBox_Params& params)
    {
        const float minTotalChildHeight = SumDesiredHeights(children);
        const float maxChildWidth = MaxDesiredWidth(children);
        const int numChildren = isize(children);

        const float xPadding = params.Padding * 2;
        const float yPadding = params.Padding * (numChildren + 1);

        return {
            .Width = maxChildWidth + xPadding,
            .Height = minTotalChildHeight + yPadding,
            .PadWidth = xPadding,
            .PadHeight = yPadding,
        };
    }

    void Layout_VertBox(const Rectangle& parentRect, std::span<ControlPtr> children, const Layout_VertBox_Params& params)
    {
        const ControlMeasurements measurements = Measure_VertBox(parentRect, children, params);
        const float availHeight = parentRect.height;
        
        const bool everythingFits = (measurements.Height) <= availHeight;
        const bool scaleToFill = (!everythingFits && params.Shrink) || (everythingFits && params.Expand);

        float scale = 1.f;
        if (scaleToFill)
            scale = (availHeight - measurements.PadHeight) / (measurements.Height - measurements.PadHeight);

        float trueY = parentRect.y + params.Padding;
        float intY = roundf(trueY);
        for (ControlPtr& ctrlP : children)
        {
            ControlState* ctrl = ctrlP.get();
            assert(ctrl);
            ctrl->Rect.x = parentRect.x;
            ctrl->Rect.width = parentRect.width;

            ctrl->Rect.y = intY;

            const float height = ctrl->MinDesiredSize.y * scale;
            trueY += height;
            intY = roundf(trueY);
            ctrl->Rect.height = intY - ctrl->Rect.y;

            trueY += params.Padding;
            intY = roundf(trueY);
        }
    }

    ControlMeasurements Measure_Control(ControlState& ctrl)
    {
        ControlMeasurements m { .Width = ctrl.MinDesiredSize.x, .Height = ctrl.MinDesiredSize.y };

        m.PadWidth = ctrl.Style->Padding.e + ctrl.Style->Padding.w;
        m.PadHeight = ctrl.Style->Padding.n + ctrl.Style->Padding.s;

        if (!ctrl.IsFixedSize && !ctrl.Children.empty())
        {
            Rectangle parentRect { .width = ctrl.MinDesiredSize.x, .height = ctrl.MinDesiredSize.y };

            switch (ctrl.LayoutParams.Algo)
            {
                case ELayoutAlgo::HorizBox:
                {
                    ControlMeasurements layoutM = Measure_HorizBox(parentRect, ctrl.Children, ctrl.LayoutParams);
                    m.Width = std::max(m.Width, layoutM.Width);
                    m.Height = std::max(m.Height, layoutM.Height);
                }
                break;

                case ELayoutAlgo::VertBox:
                {
                    ControlMeasurements layoutM = Measure_VertBox(parentRect, ctrl.Children, ctrl.LayoutParams);
                    m.Width = std::max(m.Width, layoutM.Width);
                    m.Height = std::max(m.Height, layoutM.Height);
                }
                break;

                case ELayoutAlgo::Fill:
                    break;  // trivial case
            }
        }

        return m;
    }

    
    // -------------------------------------------------------------------------------------------------
    
    void MarkDesiredSizeDirty(ControlState& ctrl)
    {
        if (ctrl.DesiredSizeDirty)
            return;

        ctrl.DesiredSizeDirty = 1;

        if (ctrl.Parent.get())
            MarkDesiredSizeDirty(*ctrl.Parent.get());

        gNeedRepaint = true;
    }

    Vector2 CalcMinDesiredOwnSize(const ControlState& ctrl)
    {
        constexpr float kDefaultMinDim = 8.f;
        FRASSERT(ctrl.Style);

        Vector2 minContentSize(kDefaultMinDim, kDefaultMinDim);
        
        if (const IconAtlas* atlas = ctrl.Icon.Atlas.get())
        {
            minContentSize = Vector2Max(minContentSize, atlas->IconSize);
        }

        if (!ctrl.Text.empty() && gCoreFont.has_value())
        {
            const int fontSize = ctrl.Style->TextSize;
            const Vector2 textSize = MeasureTextEx(*gCoreFont, ctrl.Text.c_str(), fontSize, 0.f);
            minContentSize = Vector2Max(minContentSize, textSize);
        }

        const Cardinals& pad = ctrl.Style->Padding;
        return Vector2Add(minContentSize, Vector2(pad.e + pad.w, pad.n + pad.s));
    }

    void UpdateDesiredSize(ControlState& ctrl)
    {
        if (!ctrl.DesiredSizeDirty)
            return;

        ctrl.MinDesiredSize = CalcMinDesiredOwnSize(ctrl);
        
        for (ControlPtr& childP : ctrl.Children)
        {
            ControlState* child = childP.get();
            FRASSERT(child);

            UpdateDesiredSize(*child);
        }

        ControlMeasurements m = Measure_Control(ctrl);
        ctrl.MinDesiredSize = Vector2Max(ctrl.MinDesiredSize, { m.Width, m.Height });

        ctrl.DesiredSizeDirty = false;
    }



    void RefreshControlLayout(ControlPtr& ctrlP)
    {
        ControlState* ctrl = ctrlP.get();
        FRASSERT(ctrl);
        
        UpdateDesiredSize(*ctrl);

        switch (ctrl->LayoutParams.Algo)
        {
            case ELayoutAlgo::Fill:
            {
                // just overlay all the children on the same rect
                for (ControlPtr& childP : ctrl->Children)
                {
                    ControlState* child = childP.get();
                    FRASSERT(child);
                    child->Rect = ctrl->Rect;

                    RefreshControlLayout(childP);
                }
            }
            break;

            case ELayoutAlgo::HorizBox:
            {
                Layout_HorizBox(ctrl->Rect, ctrl->Children, ctrl->LayoutParams);

                for (ControlPtr& childP : ctrl->Children)
                {
                    ControlState* child = childP.get();
                    FRASSERT(child);
                    RefreshControlLayout(childP);
                }
            }
            break;

            case ELayoutAlgo::VertBox:
            {
                Layout_VertBox(ctrl->Rect, ctrl->Children, ctrl->LayoutParams);

                for (ControlPtr& childP : ctrl->Children)
                {
                    ControlState* child = childP.get();
                    FRASSERT(child);
                    RefreshControlLayout(childP);
                }
            }
            break;

            default:
                throw std::invalid_argument("unknown layout algorithm");
        }
    }
    

    // -------------------------------------------------------------------------------------------------

    const ui::ControlPtr FindChildAt(const Vector2& pos, const ui::ControlPtr& parentP)
    {
        ui::ControlState* parent = parentP.get();
        FRASSERT(parent);
        FRASSERT(IsPointInside(pos, parent->Rect));

        for (const ui::ControlPtr& childP : parent->Children)
        {
            ui::ControlState* child = childP.get();
            FRASSERT(child);
            if (!IsPointInside(pos, child->Rect))
                continue;

            return FindChildAt(pos, childP);
        }

        return parentP;
    }

    void UpdateHoveredControl(const Vector2& mousePos)
    {
        if (gHoveredCtrl.get())
        {
            ui::ControlState* ctrl = gHoveredCtrl.get();
            if (IsPointInside(mousePos, ctrl->Rect))
            {
                if (FindChildAt(mousePos, gHoveredCtrl) == gHoveredCtrl)
                    return;
            }

            gHoveredCtrl.reset();
            ctrl->IsHovered = 0;
            gNeedRepaint = true;
        }
    
        const ControlPtr& rootCtrlP = gRootControl;
        ControlState* rootCtrl = rootCtrlP.get();
        FRASSERT(rootCtrl);
        
        if (!IsPointInside(mousePos, rootCtrl->Rect))
            return;

        gHoveredCtrl = FindChildAt(mousePos, rootCtrlP);
        FRASSERT(gHoveredCtrl.get());

        gHoveredCtrl->IsHovered = 1;
        gNeedRepaint = true;
    }
    

    void HandleInput()
    {
        UpdateHoveredControl(GetMousePosition());

        if (ui::ControlState* ctrl = gHoveredCtrl.get())
        {
            if (ui::ControlHandler* handlers = ctrl->Handlers.get())
            {
                Vector2 wndMousePos = GetMousePosition();
                Vector2 localMousePos { wndMousePos.x - ctrl->Rect.x, wndMousePos.y - ctrl->Rect.y };

                if (IsMouseButtonReleased(0))
                {
                    handlers->OnClick(gHoveredCtrl, localMousePos);
                }
                if (IsMouseButtonReleased(1))
                {
                    handlers->OnMenu(gHoveredCtrl, localMousePos);
                }
            }
        }
    }
    
    // -- rendering ---------------------------------
    bool NeedsRepaint()
    {
        return (gNeedRepaint || !gOnlyRepaintWhenDirty);
    }


    inline Color DebugColour(const ControlState& ctrl)
    {
        const float rf = ctrl.Rect.x * 13.f;
        const float gf = ctrl.Rect.y * 7.f;
        const float bf = (ctrl.Rect.width + ctrl.Rect.height) * 19.f;

        return Color { u8(int(rf)), u8(int(gf)), u8(int(bf)), 255 };
    }

    
    void RenderControl(const ui::ControlPtr& ctrlP)
    {
        const ui::ControlState* ctrl = ctrlP.get();
        FRASSERT(ctrl);

        if constexpr (gRenderDebugColouredControls)
        {
            const Color dbgCol = DebugColour(*ctrl);
            DrawRectangle(ctrl->Rect.x, ctrl->Rect.y, ctrl->Rect.width, ctrl->Rect.height, dbgCol);
        }
        else if (ctrl->Style && ctrl->Style->DrawBg)
        {
            const Color bgCol = ui::GetBgColor(*ctrl);
            DrawRectangle(ctrl->Rect.x, ctrl->Rect.y, ctrl->Rect.width, ctrl->Rect.height, bgCol);
        }

        if (ctrl->UseCustomPaint)
        {
            const ui::ControlHandler* handlers = ctrl->Handlers.get();
            FRASSERT(handlers);
            handlers->Paint(ctrlP);
            return;
        }

        Vector2 cursor = { ctrl->Rect.x, ctrl->Rect.y };
        if (ctrl->Style)
        {
            const Cardinals& pad = ctrl->Style->Padding;
            cursor.x += pad.e;
            cursor.y += pad.n;
        }
        const Color fgCol = ui::GetFgColor(*ctrl);

        if (const IconAtlas* atlas = ctrl->Icon.Atlas.get())
        {
            const Rectangle rec {
                ctrl->Icon.Location.x * atlas->IconSize.x, ctrl->Icon.Location.y * atlas->IconSize.y,
                atlas->IconSize.x, atlas->IconSize.y
            };
            
            DrawTextureRec(atlas->Texture, rec, cursor, fgCol);
        }

        if (!ctrl->Text.empty())
        {
            if (!gCoreFont.has_value())
                throw std::invalid_argument("no core ui font loaded");

            const Color fgCol = ui::GetFgColor(*ctrl);
            DrawTextEx(*gCoreFont, ctrl->Text.c_str(), cursor, gUiFontSize, 0.f, fgCol);
        }

        // draw children
        for (const ui::ControlPtr& childP : ctrl->Children)
        {
            RenderControl(childP);
        }
    }


    void Repaint()
    {
        const ControlState* rootCtrl = gRootControl.get();
        if (!rootCtrl)
        {
            ClearBackground(WHITE);
            return;
        }

        ClearBackground(rootCtrl->Style->NormalBg);

        RenderControl(gRootControl);
    }
};

