// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

/// no project includes

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <mutex>
#include <ostream>
#include <string_view>
#include <sstream>
#include <type_traits>

#define CONCAT2(x, y) x##y
#define CONCAT(x, y) CONCAT2(x, y)

#define MKVARNAME3(name, line) name ## _ ## line ## _
#define MKVARNAME2(name, line) MKVARNAME3(name, line)
#define MKVARNAME(name) MKVARNAME2(name, __LINE__)

// fixme - doesn't work for STL temporaries
#define TO_STRING(x) (static_cast<std::ostringstream&&>(std::ostringstream() << x).str())

#define ONCE(x) BRIEFLY(1, x)

#define BRIEFLY(N, ...)                                                         \
    do {                                                                        \
        static std::mutex *MKVARNAME(mut) = NEW_STATIC_OBJECT(std::mutex);      \
        static unsigned MKVARNAME(doit) = N;                                    \
                                                                                \
        std::unique_lock MKVARNAME(lock)(*MKVARNAME(mut));                      \
        if (MKVARNAME(doit)) {                                                  \
            MKVARNAME(doit)--;                                                  \
            MKVARNAME(lock).unlock();                                           \
            __VA_ARGS__;                                                        \
        }                                                                       \
    } while (false)

#define ONCEPER(t, ...)                                                         \
    do {                                                                        \
        static std::mutex *MKVARNAME(mut) = NEW_STATIC_OBJECT(std::mutex);      \
        static std::chrono::steady_clock::time_point MKVARNAME(last);           \
                                                                                \
        std::unique_lock MKVARNAME(lock)(*MKVARNAME(mut));                      \
        auto MKVARNAME(now) = std::chrono::steady_clock::now();                 \
        if (MKVARNAME(now) > MKVARNAME(last) + (t)) {                           \
            MKVARNAME(last) = MKVARNAME(now);                                   \
            MKVARNAME(lock).unlock();                                           \
            __VA_ARGS__;                                                        \
        }                                                                       \
    } while (false)

#if 0
#define WATCH(...)                                                              \
    WATCH_F((__VA_ARGS__), [](const auto &v) {                                  \
        DUMPI(#__VA_ARGS__ << ": " << v);                                       \ fixme
    })
#endif

#define WATCH_F(x, f)                                                           \
    do {                                                                        \
        static std::mutex *MKVARNAME(mut) = NEW_STATIC_OBJECT(std::mutex);      \
        static bool MKVARNAME(first) = true;                                    \
        static std::remove_reference_t<decltype(x)> MKVARNAME(cur){};           \
                                                                                \
        std::unique_lock MKVARNAME(lock)(*MKVARNAME(mut));                      \
        decltype(x) MKVARNAME(val) = x;                                         \
        if (MKVARNAME(first) || MKVARNAME(val) != MKVARNAME(cur)) {             \
            (f)(MKVARNAME(val));                                                \
            MKVARNAME(first) = false;                                           \
            MKVARNAME(cur) = MKVARNAME(val);                                    \
        }                                                                       \
    } while (false)

#define WATCH_RANGE(x, f)                                                               \
    do {                                                                                \
        static std::mutex *MKVARNAME(mut) = NEW_STATIC_OBJECT(std::mutex);              \
        static decltype(x) MKVARNAME(min) = std::numeric_limits<decltype(x)>::max();    \
        static decltype(x) MKVARNAME(max) = std::numeric_limits<decltype(x)>::lowest(); \
                                                                                        \
        std::unique_lock MKVARNAME(lock)(*MKVARNAME(mut));                              \
        decltype(x) MKVARNAME(val) = x;                                                 \
        bool MKVARNAME(changed) = false;                                                \
                                                                                        \
        if (MKVARNAME(val) < MKVARNAME(min)) {                                          \
            MKVARNAME(min) = MKVARNAME(val);                                            \
            MKVARNAME(changed) = true;                                                  \
        }                                                                               \
                                                                                        \
        if (MKVARNAME(val) > MKVARNAME(max)) {                                          \
            MKVARNAME(max) = MKVARNAME(val);                                            \
            MKVARNAME(changed) = true;                                                  \
        }                                                                               \
                                                                                        \
        if (MKVARNAME(changed)) (f)(MKVARNAME(min), MKVARNAME(max));                    \
    } while (false)

namespace hjx {

using std::ostream;
using std::string;
using std::string_view;
using std::vector;

// std::size doesn't work for compile-time array size
#define countof(x)                                                      \
    (([]{ static_assert(std::is_bounded_array_v<decltype(x)>); })(),    \
     (sizeof(x) / sizeof(*(x))))


template <class F>
constexpr void foreach_u64(const char *p, size_t n, F &&f) {
    while (n >= 8) {
        uint64_t chunk = 0;
        for (size_t i = 0; i < 8; i++) chunk = (chunk << 8) | *p++;
        f(chunk);
        n -= 8;
    }

    if (n) {
        uint64_t dregs = 0;
        for (size_t i = 0; i < n; i++) dregs = (dregs << 8) | *p++;
        f(dregs);
    }
}


inline const char *starts_with(const char *s, const char *prefix) noexcept {
    while (*prefix && *s == *prefix) { s++; prefix++; }
    return *prefix ? nullptr : s;
}

}
