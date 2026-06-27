#pragma once

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

std::string tickstring(int64_t);

namespace time {

#if 0
inline float to_seconds(clock::duration d) noexcept {
    return chrono::duration<float>(d).count();
}
#endif

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
