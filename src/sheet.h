#pragma once

#include <charconv>
#include <string>
#include <string_view>
#include <vector>

#include "fredlib.h"


// --------------------------------------------------------------------------------

namespace ui {
    using ControlPtr = std::shared_ptr<struct ControlState>;
}

// --------------------------------------------------------------------------------

struct GridSheet
{
    using StringList = std::vector<std::string>;
    using StringGrid = std::vector<StringList>;

    StringGrid RawCells;    // the backing data - what's serialised to disk

    static constexpr int kDefaultNumRows = 20;
    static constexpr int kDefaultNumCols = 20;
    static constexpr int kDefaultRowWidth = 80;

    std::vector<int> ColWidths;

    std::weak_ptr<struct GridSheetUI> UI;


    GridSheet() : ColWidths(kDefaultNumCols, kDefaultRowWidth) { /**/ }
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
// --------------------------------------------------------------------------------

struct CellIndex
{
    int Col; 
    int Row;
};

// --------------------------------------------------------------------------------

GridSheetPtr CreateNewSheet();
GridSheetUIPtr InitSheetUI(const GridSheetPtr& sheetP, ui::ControlPtr& parentCtrlP);

GridSheetPtr ChooseAndOpenSheet();

void AddRows(GridSheet& sheet, int numRowsToAdd);

// --------------------------------------------------------------------------------

// converts eg "A" to 0 and "AA" to 26
inline constexpr int ColumnNameToInt(const std::string_view s);

// converts eg 0 -> "A" and 26 -> "AA"
std::string IntToColumnName(int ix);

// converts eg "1" -> 0
int RowNameToInt(const std::string_view s);

// converts eg 0 -> "1"
std::string IntToRowName(int ix);

// converts eg "A1" to (0,0), or "$B$14" to (1,13)
CellIndex CellNameToIndex(const std::string_view s);



// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
// inline funcs
//

// converts eg "A" to 0 and "AA" to 26
inline constexpr int ColumnNameToInt(const std::string_view s)
{
    int ix = 0;

    // iterate backwards
    int offset = 0;
    int scale = 1;
    for (auto itC = s.rbegin(); itC != s.rend(); ++itC)
    {
        int c = *itC;
        if (c == '$')
            break;

        if (c >= 'a')
            c -= ('a' - 'A');

        if (c >= 'A' && c <= 'Z')
        {
            ix += (((c - 'A') + offset) * scale);
        }
        else
            throw std::invalid_argument(std::string{s});

        scale *= 26;
        offset = 1;
    }

    return ix;
}


// --------------------------------------------------------------------------------

// ZZZ is the last col we support
constexpr int MaxSheetIndex = ColumnNameToInt("ZZZ");

// --------------------------------------------------------------------------------

