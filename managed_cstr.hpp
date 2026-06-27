// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "core/pouch.hpp"
#include "core/unique_str.hpp"

#include <new>

namespace hjx {

class managed_cstr {
    const char *str;

    static auto &managed() { RETURN_STATIC_OBJECT(pouch<unique_str>); }

    managed_cstr(const char *source, bool dup) {
        if (source && dup) {
            unique_str u(source);
            source = u.get();
            managed().insert(std::move(u));
        }

        str = source;
    }

    void destroy() { if (str) managed().erase(str); }

public:
    managed_cstr(): str(nullptr) {}

    static managed_cstr dup(const char *source) { return managed_cstr(source, true); }
    static managed_cstr ref(const char *source) { return managed_cstr(source, false); }

    managed_cstr(const managed_cstr&) = delete;
    managed_cstr &operator=(const managed_cstr&) = delete;

    managed_cstr(managed_cstr &&o) noexcept: str(o.str) { o.str = nullptr; }

    managed_cstr &operator=(managed_cstr &&o) noexcept {
        if (&o != this) {
            destroy();
            str = o.str;
            o.str = nullptr;
        }

        return *this;
    }

    ~managed_cstr() { destroy(); }

    const char *ptr() const { return str; }
    operator const char*() const { return str; }
    explicit operator bool() const { return str; }

    friend ostream &operator<<(ostream &os, managed_cstr s) {
        return os << (s.str ? s.str : "");
    }
};

}
