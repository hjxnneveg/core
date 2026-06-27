// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "honest_float.hpp"
#include "reporting.hpp"

#include <cmath>
#include <numbers>

namespace hjx {

class angle {
    constexpr static float pi = std::numbers::pi_v<float>;

    constexpr static float wrap(float deg) {
        ASSERT_GE_LE(deg, -360.f, 360.f);
        if (deg > 180) return deg - 360;
        if (deg <= -180) return deg + 360;
        return deg;
    }

    constexpr static float untrusted_rad2deg720(float deg) {
        return std::remainderf(deg * (180 / pi), 360);
    }

    constexpr angle(float deg): deg_(deg) {}

public:
    // Angles are always in the interval (-180°, 180°], not overlapping.  We
    // increase not counterclockwise, but clockwise.  Absolute zero points east,
    // and ban the fucking bomb.

    // Store as degrees for fewer fp issues with hexagonally useful angles.
    // Multiples of std::numbers::pi/6 even turn into flints*.  Thanks, IEEE.
    //
    // *unless subverted by compiler flags
    // fixme - __attribute__((optimize("no-fast-math")))

    float deg_;

    constexpr angle(): deg_(0) {}

    static constexpr angle degrees(float deg) {
        return angle(wrap(deg));
    }

    static constexpr angle radians(float rad) {
        return angle(wrap(untrusted_rad2deg720(rad)));
    }

    constexpr float deg() const { return deg_; }
    constexpr float rad() const { return deg() * (pi / 180); }

    auto unit() const {
        struct { float x, y; } vec{std::cos(rad()), std::sin(rad())};
        return vec;
    }

    constexpr bool operator==(const angle &rhs) const = default;
    constexpr auto operator<=>(const angle &rhs) const = default;

    constexpr angle operator+(angle rhs) const {
        return angle::degrees(deg() + rhs.deg());
    }

    constexpr angle operator-(angle rhs) const {
        return angle::degrees(deg() - rhs.deg());
    }

    constexpr angle operator-() const {
        return angle::degrees(0 - deg());
    }

    constexpr angle operator/(const auto &rhs) const {
        ASSERT_GE(deg(), 0); // until necessary
        return angle::degrees(deg() / rhs);
    }

    constexpr angle operator*(const auto &rhs) const {
        return angle::degrees(deg() * rhs);
    }

    friend ostream &operator<<(ostream &os, angle a) {
        return os << honest_float(a.deg()) << "°";
    }
};

consteval angle operator ""_deg(long double d) {
    ASSERT_LE(d, 360);
    return angle::degrees(d);
}

consteval angle operator ""_deg(unsigned long long d) {
    ASSERT_LE(d, 360);
    return angle::degrees(d);
}

constexpr angle oclock(int hour) { return angle::degrees(hour * 30 - 90); }

}
