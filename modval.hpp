// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"
#include "typeutils.hpp"

#include <vector>

namespace hjx {

template <arithmetic Tval, arithmetic Tmod, auto op>
class modval {
    Tval value;
    std::vector<Tmod> modifiers;

public:
    modval(Tval value): value(value) {}

    void add(Tmod mod) {
        SOFTWARN_GROWTH(modifiers, 20);
        modifiers.push_back(mod);
    }

    void remove(Tmod mod) noexcept {
        for (Tmod &e : modifiers)
            if (e == mod) {
                e = std::move(modifiers.back());
                modifiers.pop_back();
                return;
            }
    }

    bool exists(Tmod mod) const noexcept {
        for (Tmod e : modifiers) if (e == mod) return true;
        return false;
    }

    Tval get() const { return op(value, modifiers); }

    Tval raw() const noexcept { return value; }
};

}
