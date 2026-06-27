// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"

namespace hjx {

class success_t {
    float v;

    void sanity_check() {
        ASSERT_NE(v, 0);
        ASSERT_GE_LE(v, -1, 1);
    }

public:
    success_t(float value): v(value) {
        sanity_check();
    }

    success_t(bool success, float value): v(success ? value : -value) {
        sanity_check();
    }

    // no implicit casts

    float get() const { return v; }
    bool is_success() const { return v > 0; }
    float success_v() const { return v > 0 ? v : 0; }
    float failure_v() const { return v < 0 ? -v : 0; }
};

}
