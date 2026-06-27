// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include <functional>
#include <ostream>
#include <utility>

namespace hjx {

class finale {
    std::function<void()> f;

public:
    finale() = default;

    finale(std::function<void()> &&f) noexcept: f(std::move(f)) {}
#if 0
    testme
    finale &operator=(finale &&o) {
        if (f) f();
        f = std::move(o.f);
        o.f = nullptr;
    }
#endif
    ~finale() { if (f) f(); }

    finale(const finale&) = delete;
    finale &operator=(const finale&) = delete;

    finale(finale &&o) noexcept: f(std::move(o.f)) { o.f = nullptr; }

    finale &operator=(finale &&o) {
        if (this != &o) {
            if (f) f(); // detonate when overwritten
            f = std::move(o.f);
            o.f = nullptr;
        }
        return *this;
    }

    bool armed() const noexcept { return bool(f); }
    void disarm() noexcept { f = nullptr; }
    void detonate() { if (f) { f(); f = nullptr; } }

    friend std::ostream &operator<<(std::ostream &os, const finale &f) {
        return os << "[" << (f.armed() ? "" : "dis")  << "armed finale]";
    }
};

}
