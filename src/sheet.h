#pragma once

#include <string>
#include <string_view>

#include "fredlib.h"


// --------------------------------------------------------------------------------

// ZZZ is the last row/col we support
// nb. 27 because "no letter" is a special extra index
constexpr int MaxSheetIndex = ((27 * 26) * 26) + 25;

// converts eg "A" to 0 and "AA" to 26
inline int SheetIndexToInt(const std::string_view s)
{
    int ix = 0;

    // iterate backwards
    int offset = 0;
    int scale = 1;
    for (auto itC = s.rbegin(); itC != s.rend(); ++itC)
    {
        int c = *itC;

        if (c == '$')
            continue;

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

// converts eg 0 -> "A" and 26 -> "AA"
inline std::string IntToSheetIndex(int ix)
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

