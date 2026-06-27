// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#pragma once

namespace hjx {

// Horner's Method

template <typename T>
class bezier2 {
    T A, B, C;

public:
    bezier2(T a, T b, T c) noexcept:
        A(c - 2*b + a), B(2*b - 2*a), C(a) {}

    T operator()(float t) const noexcept { return (A*t + B)*t + C; }
};

template <typename T>
class bezier3 {
    T A, B, C, D;

public:
    bezier3(T a, T b, T c, T d) noexcept:
        A(d - 3*c + 3*b - a), B(3*c - 6*b + 3*a), C(3*b - 3*a), D(a) {}

    T operator()(float t) const noexcept { return ((A*t + B)*t + C)*t + D; }
};

}
