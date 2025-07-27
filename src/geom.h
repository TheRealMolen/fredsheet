#pragma once

#include "raylib.h"


// --------------------------------------------------------------------------------

inline bool IsPointInside(const Vector2& pt, const Rectangle& rect)
{
    if (pt.x < rect.x || pt.y < rect.y)
        return false;
    if (pt.x >= (rect.x + rect.width) || (pt.y >= rect.y + rect.height))
        return false;

    return true;
}

// --------------------------------------------------------------------------------


