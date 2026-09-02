// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

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
