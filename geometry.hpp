// SPDX-License-Identifier: UNLICENSED
// Copyright (c) 2025-2026 Joshua Marshall
// All rights reserved.

#pragma once

#include "core/coords.hpp"

#include "core/logging.hpp"

namespace hjx::geom {

// guardrails for now
constexpr float min_fence = 0.0001;
constexpr float max_fence = 1000;

class circle {
    xy center_;
    float radius_;

public:
    circle(xy center, float radius): center_(center), radius_(radius) {
        ASSERT_LT(center.abs(), max_fence);
        ASSERT_GT_LT(radius, min_fence, max_fence);
    }

    xy center() const { return center_; }
    float radius() const { return radius_; }

    friend std::ostream &operator<<(std::ostream &os, const circle &circ) {
        return os << "[circle " << circ.center() << " " << circ.radius() << "]";
    }
};

// distance along path [0, 1] else NaN
inline float intersect(xy from, xy to, const circle &target) {
    float R = target.radius();
    xy delta = to - from;
    xy f = from - target.center();
    float a = delta.autodot();

    // zeroish-length sweep: static overlap test
    if (a < 1e-12f) return f.autodot() <= R*R ? 0.f : nan::make();

    float b = 2 * f.dot(delta);
    float c = f.autodot() - R*R;
    float disc = b*b - 4*a*c;
    if (disc < 0) return nan::make();

    disc = std::sqrt(disc);
    float t = (-b - disc) / (2 * a);

    if (t > 1) return nan::make();

    // started inside or behind
    if (t < 0) return c <= 0 ? 0.f : nan::make();

    return t;
}

}
