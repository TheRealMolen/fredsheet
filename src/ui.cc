#include "ui.h"

#include "raymath.h"

#include <cmath>
#include <numeric>
#include <optional>
#include <span>


namespace ui
{
    // functional switches
    constexpr bool gOnlyRepaintWhenDirty = false;
    constexpr bool gRenderDebugColouredControls = true;


    int gUiFontSize = kDefaultFontSize;
    std::optional<Font> gCoreFont;

    const ControlStyle kDefaultControlStyle;
    const ControlStyle kNullControlStyle = { .DrawBg = 0 };
    
    ControlPtr gRootControl;

    static ui::ControlPtr gHoveredCtrl;

    static bool gNeedRepaint = true;



    void LoadUIFont()
    {
        gCoreFont = LoadFontEx("C:\\Windows\\Fonts\\calibril.ttf", gUiFontSize, nullptr, 0);
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


    ControlPtr CreateRootControl(int width, int height)
    {
        FRASSERT(!gRootControl.get());

        ui::ControlPtr ctrl = make_shared<ui::ControlState>("ROOT");
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

    ControlMeasurements Measure_HorizBox(const Rectangle& parentRect, std::span<const ControlPtr> children, const Layout_HorizBox_Params& params)
    {
        const float minTotalChildWidth = std::accumulate(begin(children), end(children), 0.f, [](float acc, const auto& ctrlP) { return (acc + ctrlP->MinDesiredSize.x); });
        const int numChildren = isize(children);

        const float xPadding = params.Padding * (numChildren + 1);
        const float yPadding = params.Padding * 2;

        return {
            .Width = minTotalChildWidth + xPadding,
            .Height = parentRect.height + yPadding,
            .PadWidth = xPadding,
            .PadHeight = yPadding,
        };
    }

    void Layout_HorizBox(const Rectangle& parentRect, std::span<ControlPtr> children, const Layout_HorizBox_Params& params)
    {
        const ControlMeasurements measurements = Measure_HorizBox(parentRect, children, params);
        const float availWidth = parentRect.width;
        
        const bool everythingFits = measurements.Width <= availWidth;
        float scale = 1.f;
        if (everythingFits || !params.Shrink)
        {
            if (everythingFits && params.Expand)
            {
                scale = (availWidth - measurements.PadWidth) / (measurements.Width - measurements.PadWidth);
            }
        }
        else
        {
            scale = (availWidth - measurements.PadWidth) / (measurements.Width - measurements.PadWidth);
        }

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
        const float minTotalChildHeight = std::accumulate(begin(children), end(children), 0.f, [](float acc, const auto& ctrlP) { return (acc + ctrlP->MinDesiredSize.y); });
        const int numChildren = isize(children);

        const float xPadding = params.Padding * 2;
        const float yPadding = params.Padding * (numChildren + 1);

        return {
            .Width = parentRect.width + xPadding,
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
        float scale = 1.f;
        if (everythingFits || !params.Shrink)
        {
            if (everythingFits && params.Expand)
            {
                scale = (availHeight - measurements.PadHeight) / (measurements.Height - measurements.PadHeight);
            }
        }
        else
        {
            scale = (availHeight - measurements.PadHeight) / (measurements.Height - measurements.PadHeight);
        }

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

    ControlMeasurements Measure_Control(const ControlPtr& ctrlP)
    {
        const ControlState* ctrl = ctrlP.get();
        FRASSERT(ctrl);

        ControlMeasurements m { .Width = ctrl->MinDesiredSize.x, .Height = ctrl->MinDesiredSize.y };

        if (!ctrl->IsFixedSize && !ctrl->Children.empty())
        {
            Rectangle parentRect { .width = ctrl->MinDesiredSize.x, .height = ctrl->MinDesiredSize.y };

            switch (ctrl->LayoutParams.Algo)
            {
                case ELayoutAlgo::HorizBox:
                {
                    ControlMeasurements layoutM = Measure_HorizBox(parentRect, ctrl->Children, ctrl->LayoutParams);
                    m.Width = std::max(m.Width, layoutM.Width);
                }
                break;

                case ELayoutAlgo::VertBox:
                {
                    ControlMeasurements layoutM = Measure_VertBox(parentRect, ctrl->Children, ctrl->LayoutParams);
                    m.Height = std::max(m.Height, layoutM.Height);
                }
                break;

                case ELayoutAlgo::Fill:
                    break;  // trivial case
            }
        }

        m.PadWidth = ctrl->Style->Padding.e + ctrl->Style->Padding.w;
        m.PadHeight = ctrl->Style->Padding.n + ctrl->Style->Padding.s;
        m.Width += m.PadWidth;
        m.Height += m.PadHeight;

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
    

    void TryDispatchMouseClick()
    {
        if (!gHoveredCtrl.get())
            return;

        ui::ControlHandlerPtr handlerP = gHoveredCtrl->Handlers;
        if (handlerP.get())
        {
            handlerP->OnClick(gHoveredCtrl);
        }
    }
    
    void HandleInput()
    {
        UpdateHoveredControl(GetMousePosition());

        if (IsMouseButtonReleased(0))
        {
            TryDispatchMouseClick();
        }
    }
    
    // -- rendering ---------------------------------
    bool NeedsRepaint()
    {
        return (gNeedRepaint || !gOnlyRepaintWhenDirty);
    }


    inline Color DebugColour(const ControlState& ctrl)
    {
        const float rf = ctrl.Rect.x * 10.f;
        const float gf = ctrl.Rect.y * 10.f;
        const float bf = (ctrl.Rect.width + ctrl.Rect.height) * 10.f;

        return Color { u8(int(rf)), u8(int(gf)), u8(int(bf)), 255 };
    }

    
    void RenderControl(const ui::ControlPtr& ctrlP)
    {
        const ui::ControlState* ctrl = ctrlP.get();
        FRASSERT(ctrl);

        if (ctrl->Style && ctrl->Style->DrawBg)
        {
            const Color bgCol = ui::GetBgColor(*ctrl);
            DrawRectangle(ctrl->Rect.x, ctrl->Rect.y, ctrl->Rect.width, ctrl->Rect.height, bgCol);
        }
        else if constexpr (gRenderDebugColouredControls)
        {
            const Color dbgCol = DebugColour(*ctrl);
            DrawRectangle(ctrl->Rect.x, ctrl->Rect.y, ctrl->Rect.width, ctrl->Rect.height, dbgCol);
        }

        for (const ui::ControlPtr& childP : ctrl->Children)
        {
            RenderControl(childP);
        }

        if (!ctrl->Text.empty())
        {
            if (!gCoreFont.has_value())
                throw std::invalid_argument("no core ui font loaded");

            Vector2 cursor = { ctrl->Rect.x, ctrl->Rect.y };
            if (ctrl->Style)
            {
                const Cardinals& pad = ctrl->Style->Padding;
                cursor.x += pad.e;
                cursor.y += pad.n;
            }

            const Color fgCol = ui::GetFgColor(*ctrl);
            DrawTextEx(*gCoreFont, ctrl->Text.c_str(), cursor, gUiFontSize, 0.f, fgCol);
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

