// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"

#include <charconv>
#include <ostream>
#include <string_view>

namespace hjx {

// because floats ostream wrong
class honest_float {
    float storage;

public:
    constexpr honest_float(float x): storage(x) {}

    honest_float(std::string_view s) {
        IF_ASSERTS_ON(auto [ptr, ec] =)
            std::from_chars(s.data(), s.data() + s.size(), storage);
        ASSERT_MSG(ec == std::errc{}, s);
        ASSERT_EQ(ptr, s.data() + s.size());
    }

    constexpr operator float() const { return storage; }

    friend std::ostream &operator<<(std::ostream &os, honest_float x) {
        char buf[16];
        auto [ptr, ec] = std::to_chars(buf, buf + 15, x.storage);
        ASSERT(ec == std::errc{});
        ASSERT_LE((void*)ptr, (void*)&buf[15]);
        *ptr = '\0';
        return os << buf;
    }
};

}
