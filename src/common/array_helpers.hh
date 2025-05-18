#ifndef __ARRAY_HELPERS_HH
#define __ARRAY_HELPERS_HH

#include <array>
#include <algorithm>
#include <cstddef>

template<typename t, size_t n>
constexpr std::array<t, n> init_array(t value) {
    auto array = std::array<t, n>{};
    std::fill(array.begin(), array.end(), value);
    return array;
}

#endif