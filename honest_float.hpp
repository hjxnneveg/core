// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"

#include <charconv>
#include <limits>
#include <ostream>
#include <string_view>
#include <system_error>

namespace hjx {

// because floats ostream wrong
class honest_float {
    float storage;

public:
    constexpr honest_float(float x): storage(x) {}

    explicit honest_float(std::string_view s) {
        storage = std::numeric_limits<float>::quiet_NaN();
        IF_ASSERTS_ON(auto [ptr, ec] =)
            std::from_chars(s.data(), s.data() + s.size(), storage);
        ASSERT_MSG(ec == std::errc{}, s);
        ASSERT_EQ(ptr, s.data() + s.size());
    }

    constexpr operator float() const { return storage; }

    friend std::ostream &operator<<(std::ostream &os, honest_float x) {
        char buf[20];
        auto [ptr, ec] = std::to_chars(buf, buf + 15, x.storage);
        ASSERT(ec == std::errc{});
        ASSERT_LE((void*)ptr, (void*)&buf[15]);
        *ptr = '\0';
        return os << buf;
    }
};

class as_hex {
    uint64_t storage;

public:
    explicit as_hex(uint64_t u): storage(u) {}

    operator uint64_t() const { return storage; }

    friend std::ostream &operator<<(std::ostream &os, as_hex u) {
        static constexpr char digits[] = "0123456789abcdef";

        char buf[21] = "0x";
        char *p = buf + 2;

        for (int i = 15; i >= 0; i--) {
            *p++ = digits[(u.storage >> (i * 4)) & 0xf];
            if (i % 4 == 0 && i != 0) *p++ = '\'';
        }

        // Inserting a string_view still honors os.width() and os.fill(), so
        // `os << std::setw(24) << as_hex(x)` pads the whole token as expected.
        return os << std::string_view(buf, sizeof buf);
    }
};

}
