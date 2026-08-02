// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#pragma once

#include "honest_float.hpp"
#include "reporting.hpp"

#include <bit>
#include <cmath>
#include <cstdint>
#include <ostream>
#include <type_traits>

namespace hjx {

// snip off the sign bit and 15 mantissa bits
// non-negative, finite
// ε = 1/256 = 0.00390625
class weefloat {
    // rounds down--don't tell any statisticians
    static constexpr uint16_t encode(float x) {
        ASSERT_MSG(std::isfinite(x), honest_float{x} << " non-finite");
        ASSERT_GE(x, 0); // accepts -0.f
        return uint16_t(std::bit_cast<uint32_t>(x) >> 15);
    }

    static constexpr float decode(uint16_t u) {
        return std::bit_cast<float>(uint32_t(u) << 15);
    }

    uint16_t storage;

public:
    static constexpr weefloat epsilon() { return weefloat{1/256.f}; }

    // NB: uninitialized by default, no nil value
    constexpr weefloat() = default;

    constexpr explicit weefloat(float x): storage(encode(x)) {}

    // strong ordering and int comparison (no nans, no negatives)
    constexpr bool operator==(const weefloat&) const = default;
    constexpr auto operator<=>(const weefloat&) const = default;

    // NB: floats truncate before comparison
    friend constexpr bool operator==(weefloat a, float b) {
        return a.storage == encode(b);
    }

    friend constexpr auto operator<=>(weefloat a, float b) {
        return a.storage <=> encode(b);
    }

    constexpr operator float() const { return decode(storage); }

    constexpr auto raw_() const { return storage; }

    friend std::ostream &operator<<(std::ostream &os, weefloat v) {
        return os << honest_float{v};
    }
};

static_assert(sizeof(weefloat) == 2);
static_assert(std::is_trivially_default_constructible_v<weefloat>);
static_assert(std::is_trivially_copyable_v<weefloat>);
static_assert(std::is_trivially_copy_constructible_v<weefloat>);
static_assert(std::is_trivially_move_constructible_v<weefloat>);
static_assert(std::is_standard_layout_v<weefloat>);

}
