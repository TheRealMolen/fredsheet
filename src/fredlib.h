#pragma once

#include "debug/assert.h"

#include <cstdint>
#include <memory>

using u8 = uint8_t;
using u32 = uint32_t;


#define FRASSERT    _ASSERT



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



// workaround for 10x parser not supporting inline namespaces
#ifdef FREDSHEET_10x

namespace std
{
    using weak_ptr = _LIBCPP_ABI_NAMESPACE::weak_ptr;
    using shared_ptr = _LIBCPP_ABI_NAMESPACE::shared_ptr;

    inline auto make_shared(auto&&... args)
    {
        return _LIBCPP_ABI_NAMESPACE::make_shared(args);
    }


    using string = _LIBCPP_ABI_NAMESPACE::string;
    using string_view = _LIBCPP_ABI_NAMESPACE::string_view;
    using vector = _LIBCPP_ABI_NAMESPACE::vector;
};

#endif
