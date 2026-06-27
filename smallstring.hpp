// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "reporting.hpp"

#include <charconv>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <ostream>
#include <string_view>

namespace hjx {

template <size_t N>
class smallstring {
public:
    // conservative assertion to catch cases like smallstring<16>,
    // which would step into an additional word because of the nul.
    // safe to remove.
    // nicely-aligned cases are smallstring<7 + 8ℤ>.
    static_assert((N + 1) % 8 == 0);

    static constexpr size_t capacity = N;

private:
    char storage[capacity + 1];

    char &headroom() { return storage[capacity]; }
    const char &headroom() const { return storage[capacity]; }

public:
    smallstring() {
        storage[0] = '\0';
        headroom() = capacity;
    }

    smallstring(std::string_view s) {
        size_t sz = s.size();
        ASSERT_LE(sz, capacity);
        memcpy(storage, s.data(), sz);
        storage[sz] = '\0';
        headroom() = capacity - sz;
    }

    smallstring(std::unsigned_integral auto x) {
        uint64_t z = x;
        ASSERT_GE(capacity, 18);
        storage[0] = '0';
        storage[1] = 'x';
        for (int i = 0; i < 16; i++) {
            uint8_t d = z >> (60 - i * 4) & 0xf;
            storage[i + 2] = (d > 9 ? 'a' - 10 : '0') + d;
        }
        storage[18] = '\0';
        headroom() = capacity - 18;
    }

    smallstring(std::same_as<float> auto x) {
        auto [ptr, ec] = std::to_chars(data(), data() + capacity, x);
        ASSERT(ec == std::errc{});
        ASSERT_LE((void*)ptr, (void*)(data() + capacity));
        *ptr = '\0';
        headroom() = capacity - (ptr - data());
    }

    smallstring &operator=(const auto &o) { return *this = smallstring(o); }

    const char *data() const { return storage; }
    char *data() { return storage; }
    size_t size() const { return capacity - headroom(); }
    bool full() const { return !headroom(); }

    std::string_view sv() const { return {data(), size()}; }
    operator std::string_view() const { return sv(); }

    bool operator==(const smallstring &o) const { return sv() == o.sv(); }
    auto operator<=>(const smallstring &o) const { return sv() <=> o.sv(); }
    bool operator==(std::string_view o) const { return sv() == o; }
    auto operator<=>(std::string_view o) const { return sv() <=> o; }

    void append(char c) {
        ASSERT_LT(size(), capacity);
        size_t sz = size();
        storage[sz] = c;
        headroom()--;
        storage[sz + 1] = '\0';
    }

    friend std::ostream &operator<<(std::ostream &os, const smallstring &s) {
        return os << s.data();
    }
};

}
