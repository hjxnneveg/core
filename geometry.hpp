// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "core/coords.hpp"

#include "core/logging.hpp"

namespace hjx::geom {

// guardrails for now
constexpr float min_fence = 0.0001;
constexpr float max_fence = 1000;

constexpr float zeroish = 1e-8;

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
inline float intersect(xy obj, xy delta, const circle &target) {
    float R = target.radius();
    xy f = obj - target.center();
    float a = delta.autodot();

    // zeroish-length sweep: static overlap test
    if (a <= zeroish*zeroish) return f.autodot() <= R*R ? 0.f : nan::make();

    float h = f.dot(delta);
    float c = f.autodot() - R*R;
    float disc = h*h - a*c; // (b²-4ac)/4
    if (disc < 0) return nan::make();

    float t = (-h - std::sqrt(disc)) /  a;
    if (t > 1) return nan::make();

    // started inside or behind
    if (t < 0) return c <= 0 ? 0.f : nan::make();

    return t;
}

struct intersection {
    xy impact{};
    xy normal{};
    float t = nan::make();

    explicit operator bool() const { return bool(impact); }

    friend std::ostream &operator<<(std::ostream &os, const intersection &meet) {
        if (!meet) return os << "nil";
        return os << "[" << meet.impact
                  << " t=" << honest_float(meet.t)
                  << " ⟂" << meet.normal << "]";
    }
};

inline intersection intersect(const circle &obj, xy delta, const circle &target) {
    circle inflated{target.center(), obj.radius() + target.radius()};
    float t = intersect(obj.center(), delta, inflated);
    if (std::isnan(t)) return intersection{};

    xy displaced = obj.center() + delta * t;
    xy offset = displaced - target.center();
    xy normal = offset.autodot() <= zeroish ? -delta.normalize() : offset.normalize();

    return {
        .impact = target.center() + normal * target.radius(),
        .normal = normal,
        .t = t
    };
}

}
