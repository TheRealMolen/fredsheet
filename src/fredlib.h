#pragma once

#include "debug/assert.h"

#include <cstdint>
#include <memory>

using u8 = uint8_t;
using u32 = uint32_t;


#define FRASSERT    _ASSERT


using std::begin, std::end, std::size;;

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



// workaround for 10x parser not supporting inline namespaces
#ifdef FREDSHEET_10x
namespace std
{
    using namespace _LIBCPP_ABI_NAMESPACE;

    using string = _LIBCPP_ABI_NAMESPACE::basic_string<char>;
    using string_view = _LIBCPP_ABI_NAMESPACE::basic_string_view<char>;
};
#endif
