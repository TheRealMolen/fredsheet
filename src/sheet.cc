
#include "sheet.h"

#include "fredlib.h"
#include "ui.h"


// --------------------------------------------------------------------------------

GridSheetUIPtr InitSheetUI(const GridSheetPtr& sheetP, ui::ControlPtr& parentCtrlP)
{
    FRASSERT(parentCtrlP.get());

    GridSheet* sheet = sheetP.get();
    FRASSERT(sheet);
    FRASSERT(sheet->UI.expired());

    GridSheetUIPtr uiP = std::make_shared<GridSheetUI>();
    GridSheetUI* ui = uiP.get();

    sheet->UI = uiP;
    ui->Sheet = sheetP;

    ui->ParentCtrl = parentCtrlP;

    constexpr float kRowTitleWidth = 40.f;
    constexpr float kPadding = 2.f;
    constexpr float kSpacing = 1.f;
    const float kRowHeight = ui::MeasureTextHeight(ui::kDefaultFontSize) + 2.f * kPadding;

    const Rectangle parentRect = parentCtrlP->Rect;

    // set up column headers
    float colTop = parentRect.y;
    float colLeft = parentRect.x + kRowTitleWidth + kSpacing;
    for (int col = 0; col < isize(sheet->ColWidths); ++col)
    {
        const float colWidth = float(sheet->ColWidths[col]);
        ui::ControlPtr colHdr = make_shared<ui::ControlState>("col-hdr", IntToColumnName(col));
        colHdr->Rect = Rectangle{ colLeft, colTop, colWidth, kRowHeight };
        colHdr->Style = &ui::kTightControlStyle;
        ui::AddChild(parentCtrlP, colHdr);

        colLeft += colWidth +  kSpacing;
    }
    
    // set up row headers
    float rowTop = parentRect.y + kRowHeight + kSpacing;
    float rowLeft = parentRect.x;
    for (int row = 0; row < isize(sheet->RawCells); ++row)
    {
        ui::ControlPtr rowHdr = make_shared<ui::ControlState>("row-hdr", IntToRowName(row));
        rowHdr->Rect = Rectangle{ rowLeft, rowTop, kRowTitleWidth, kRowHeight };
        rowHdr->Style = &ui::kTightControlStyle;
        ui::AddChild(parentCtrlP, rowHdr);

        rowTop += kRowHeight + kSpacing;
    }
    
    return uiP;
}

// --------------------------------------------------------------------------------

GridSheetPtr CreateNewSheet()
{
    GridSheetPtr sheetP = std::make_shared<GridSheet>();
    GridSheet* sheet = sheetP.get();
    FRASSERT(sheet);

    const int numCols = isize(sheet->ColWidths);
    
    sheet->RawCells.reserve(GridSheet::kDefaultNumRows);
    for (int row = 0; row < GridSheet::kDefaultNumRows; ++row)
    {
        sheet->RawCells.emplace_back(numCols);
    }

    return sheetP;
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

int RowNameToInt(const std::string_view s)
{
    int ix = -1;

    const char* first = s.data();
    const char* last = s.data() + s.size();
    auto res = std::from_chars(first, last, ix, 10);

    if (res.ec != std::errc() || (res.ptr != last))
        throw std::invalid_argument(std::string{s});

    if (ix < 1)
        throw std::invalid_argument(std::string{s});

    return ix - 1;  // nb. rows are 1-indexed
}

// --------------------------------------------------------------------------------

CellIndex CellNameToIndex(const std::string_view s)
{
    const auto itBegin = begin(s);
    const auto itEnd = end(s);
    auto itColBegin = itBegin;

    // scan to find the start of the column
    for (; itColBegin != itEnd; ++itColBegin)
    {
        const char c = *itColBegin;
        if (c >= 'A' && c <= 'Z')
            break;
    }
    // now keep going till the end of the column
    auto itColEnd = itColBegin;
    for (; itColEnd != itEnd; ++itColEnd)
    {
        const char c = *itColEnd;
        if (c < 'A' || c > 'Z')
            break;
    }

    // skip any $ or other paraphernalia
    auto itRowBegin = itColEnd;
    for (; itRowBegin != itEnd; ++itRowBegin)
    {
        const char c = *itColEnd;
        if (c >= '0' && c <= '9')
            break;
    }

    if (itColEnd == itBegin || itRowBegin == itEnd)
        throw std::invalid_argument(std::string(s));

    CellIndex ix {
        .Col = ColumnNameToInt( { itColBegin, itColEnd } ),
        .Row = RowNameToInt( { itRowBegin, itEnd }),
    };
    return ix;
}

// --------------------------------------------------------------------------------

// converts eg 0 -> "A" and 26 -> "AA"
std::string IntToColumnName(int ix)
{
    FRASSERT(ix >= 0);
    FRASSERT(ix <= MaxSheetIndex);
    if (ix < 26)
        return std::string(1, char('A' + ix));

    ix -= 26;
    if (ix < (26 * 26))
    {
        std::string s;
        s.reserve(2);
        const int topIx = (ix / 26) % 26;
        const int btmIx = ix % 26;
        s.push_back('A' + topIx);
        s.push_back('A' + btmIx);
        return s;
    }

    ix -= (26 * 26);
    std::string s;
    s.reserve(3);
    const int topIx = (ix / (26*26)) % 26;
    const int midIx = (ix / 26) % 26;
    const int btmIx = ix % 26;
    s.push_back('A' + topIx);
    s.push_back('A' + midIx);
    s.push_back('A' + btmIx);
    return s;
}

// --------------------------------------------------------------------------------

// converts eg 0 -> "1"
std::string IntToRowName(int ix)
{
    FRASSERT(ix >= 0);
    return std::to_string(ix + 1);
}

// --------------------------------------------------------------------------------


