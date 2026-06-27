// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "honest_float.hpp"
#include "utf.hpp"
#include "reporting.hpp"
#include "mathbits.hpp"

#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ostream>
#include <type_traits>

// decimal representation for user-visible values
// not a full numeric type

namespace hjx {

// todo - std::pow constexpr in 26, because why not half-ass it in 23?
consteval int64_t pow10c(unsigned n) {
    int64_t r = 1;
    for (unsigned i = 0; i < n; ++i) r *= 10;
    return r;
}

template<std::integral Rep, unsigned Exp>
requires (sizeof(Rep) <= 2) // until needed (double API)
struct decimal {
    constexpr static Rep min_repr = [] {
        if constexpr (std::is_unsigned_v<Rep>)
            return std::numeric_limits<Rep>::min();
        else
            return std::numeric_limits<Rep>::min() + 1;
    }();

    constexpr static Rep max_repr = [] {
        if constexpr (std::is_unsigned_v<Rep>)
            return std::numeric_limits<Rep>::max() - 1;
        else
            return std::numeric_limits<Rep>::max();
    }();

    // nil is not nan
    static constexpr Rep nil_repr = [] {
        if constexpr (std::is_unsigned_v<Rep>)
            return std::numeric_limits<Rep>::max();
        else
            return std::numeric_limits<Rep>::min();
    }();

    static constexpr int64_t scale = pow10c(Exp);

    constexpr static Rep fix(float x) {
        if (std::isnan(x)) return nil_repr;
        x = std::round(x * scale);
        if (x >= min_repr && x <= max_repr) return static_cast<Rep>(x);
        return nil_repr;
    }

    constexpr static float unfix(Rep v) {
        if (v == nil_repr) return nan::make();
        return static_cast<float>(v) / scale;
    }

    Rep storage;

    constexpr decimal(): storage(nil_repr) {}

    constexpr decimal(float x): storage(fix(x)) {}

    static constexpr decimal from_repr(Rep r) {
        decimal ret;
        ret.storage = r;
        return ret;
    }

    constexpr honest_float get() const { return unfix(storage); }

    constexpr bool valid() const { return storage != nil_repr; }
    constexpr bool nilp() const { return storage == nil_repr; }

    constexpr Rep repr() const { return storage; }

    constexpr bool operator==(const decimal&) const = default;

    constexpr auto operator<=>(decimal o) const {
        if (storage == o.storage) return std::partial_ordering::equivalent;
        if (nilp() || o.nilp()) return std::partial_ordering::unordered;
        if (storage < o.storage) return std::partial_ordering::less;
        return std::partial_ordering::greater;
    }

    static constexpr decimal min() { return from_repr(min_repr); }
    static constexpr decimal max() { return from_repr(max_repr); }
    static constexpr decimal nil() { return from_repr(nil_repr); }

    // no parsable text representation for nil
    // i.e. nil return => parse failure with input feed left untouched
    static decimal parse(utf::feed &f) {
        utf::feed x = f;

        int64_t sign = x.skipspace().consume('-') ? -1 : 1;
        if constexpr (std::is_unsigned_v<Rep>)
            if (sign == -1) return {};

        bool digits = false;
        int64_t value = 0;

        auto whole = x.parse_uint();
        if (whole) {
            if (*whole > int64max / scale) return {};
            value = static_cast<int64_t>(*whole) * scale;
            digits = true;
        }

        if (x.consume('.')) {
            uint64_t rank = scale;
            while (x.peek().isdigit()) {
                if (rank == 1) return {};
                rank /= 10;
                ASSERT(rank);
                value += rank * (x.next() - '0');
                digits = true;
            }
        }

        if (!digits || value > max_repr) return {};
        f = x;
        return from_repr(static_cast<Rep>(value * sign));
    }

    static decimal parse(std::string_view s) {
        utf::feed x{s};
        return parse(x);
    }

    friend std::ostream &operator<<(std::ostream &os, decimal v) {
        if (v.nilp()) return os << "nil";
        return os << v.get();
    }
};

using udec16 = decimal<uint16_t, 1>; // [0, 6553.4]
using cent16 = decimal<int16_t,  2>; // [-327.67, 327.67]

}
