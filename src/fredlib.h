#pragma once

#include <cassert>
#include <cstdint>

using u8 = uint8_t;
using u32 = uint32_t;


#define FRASSERT    assert



using std::begin, std::end, std::size;
using std::weak_ptr, std::shared_ptr, std::make_shared;

inline int isize(const auto& cont)
{
    return int(cont.size());
}


struct Cardinals
{
    float n;
    float e;
    float s;
    float w;

    Cardinals(float all) : n(all), e(all), s(all), w(all) { /**/ }
    Cardinals(float ns, float ew) : n(ns), e(ew), s(ns), w(ew) { /**/ }
    Cardinals(float n, float e, float s, float w) : n(n), e(e), s(s), w(w) { /**/ }
};



inline bool IsPointInside(const Vector2& pt, const Rectangle& rect)
{
    if (pt.x < rect.x || pt.y < rect.y)
        return false;
    if (pt.x >= (rect.x + rect.width) || (pt.y >= rect.y + rect.height))
        return false;

    return true;
}
