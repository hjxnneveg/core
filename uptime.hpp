// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "reporting.hpp"
#include "time.hpp"

#include <ostream>

namespace hjx::time {

class uptime_t {
    duration d_;

public:
    uptime_t(duration d) NOTHROW: d_(d) { ASSERT(d >= duration()); }

    operator duration() const noexcept { return d_; }

    uint64_t nanos() const noexcept {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(d_).count();
    }

    float seconds() const noexcept {
        return std::chrono::duration<float>(d_).count();
    }

    friend std::ostream &operator<<(std::ostream &os, uptime_t t) {
        return os << tickstring(t.d_.count());
    }
};

inline uptime_t operator+(uptime_t t, duration d) NOTHROW {
    return uptime_t(duration(t) + d);
}

inline uptime_t operator+(duration d, uptime_t t) { return t + d; }


clock::time_point game_start();

void pause(/*uint64_t player_id*/);
void unpause();
bool paused();

uptime_t uptime();
float uptime_seconds();

}
