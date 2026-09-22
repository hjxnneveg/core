// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "reporting.hpp"
#include "statics.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace hjx {

using namespace std::chrono_literals;

namespace chrono = std::chrono;

using clock = chrono::steady_clock;
using time_point = clock::time_point;
using duration = clock::duration;
static_assert(sizeof(duration::rep) == 8);
static_assert(std::is_same_v<duration::period, std::nano>);

inline std::string tickstring(int64_t n) {
    ASSERT_GE(n, 0);

    int64_t ks = n / 1'000'000'000'000;
    int64_t  s = n / 1'000'000'000 % 1000;
    int64_t ms = n / 1'000'000 % 1000;
    int64_t us = n / 1'000 % 1000;
    int64_t ns = n % 1000;

    std::ostringstream ss;
    ss << std::setfill('0') << '[';
    ss << std::setw(3) << ks << '\'';
    ss << std::setw(3) <<  s << '\'';
    ss << std::setw(3) << ms << '\'';
    ss << std::setw(3) << us << '\'';
    ss << std::setw(3) << ns << ']';
    return ss.str();
}

namespace time {

inline std::string format_now() {
    chrono::system_clock::time_point now = chrono::system_clock::now();
    std::time_t t = chrono::system_clock::to_time_t(now);
    std::tm *tm_info = std::localtime(&t);
    std::ostringstream ss;
    ss << std::put_time(tm_info, "%T");
    return ss.str();
}

inline uint64_t now_ns() { return clock::now().time_since_epoch().count(); }

}

inline std::ostream &operator<<(std::ostream &os, const duration &d) {
    if (d.count() < 100'000) return os << d.count() << "ns";
    if (d.count() < 100'000'000) return os << d.count() / 1000 << "µs";
    return os << d.count() / 1'000'000 << "ms";
}

constexpr duration operator*(duration dur, float f) noexcept {
    return duration(static_cast<duration::rep>(f * dur.count()));
}

constexpr duration operator*(duration dur, double f) noexcept {
    return duration(static_cast<duration::rep>(f * dur.count()));
}

constexpr duration operator*(float f, duration dur) noexcept {
    return duration(static_cast<duration::rep>(f * dur.count()));
}

constexpr duration operator*(double f, duration dur) noexcept {
    return duration(static_cast<duration::rep>(f * dur.count()));
}

template <typename Rep, typename Period>
constexpr float to_seconds(chrono::duration<Rep, Period> d) noexcept {
    return duration_cast<chrono::duration<float, std::ratio<1, 1>>>(d).count();
}

}
