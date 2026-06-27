// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"

#include <cstdlib>
#include <cstring>
#include <string_view>

namespace hjx {

class unique_str {
    char *str;

public:
    unique_str(): str(nullptr) {}

    unique_str(std::string_view s): str(static_cast<char*>(std::malloc(s.size() + 1))) {
#if 1 // consider DIAGNOSTIC macros
        if (!str) ERROR("malloc failure");
#endif
        std::memcpy(str, s.data(), s.size());
        str[s.size()] = '\0';
    }

    unique_str(unique_str &&o) noexcept: str(o.str) { o.str = nullptr; }

    unique_str &operator=(unique_str &&o) noexcept {
        if (&o != this) {
            std::free(str);
            str = o.str;
            o.str = nullptr;
        }

        return *this;
    }

    ~unique_str() { std::free(str); }

    unique_str(const unique_str &o) = delete;
    unique_str &operator=(const unique_str &o) = delete;

    operator const char*() const { return str; }
    const char *get() const { return str; }

    friend std::ostream &operator<<(std::ostream &os, const unique_str &s) {
        return os << (s.get() ? s.get() : "<null>");
    }
};

}
