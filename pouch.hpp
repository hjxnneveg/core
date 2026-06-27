// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"

#include <cstddef>
#include <vector>

namespace hjx {

// O(n) behavior.  Use for small collections.
// Avoids conversions to T when possible.

template <typename T>
class pouch {
    std::vector<T> storage;

public:
    static constexpr size_t soft_limit = 50;

    void insert(auto &&v) {
        // consider
        // DIAGNOSTIC(pouch,
        //            sz == soft_limit || sz > soft_limit && sz == cap,
        //            "pouch exceeds " << sz << " elements");
        SOFTWARN_GROWTH(storage, soft_limit);

        storage.push_back(std::forward<decltype(v)>(v));
    }

    void erase(const auto &v) {
        for (size_t i = storage.size(); i--;)
            if (storage[i] == v) {
                storage[i] = std::move(storage.back());
                storage.pop_back();
                return;
            }
    }

    bool contains(const auto &v) const {
        for (const T &e : storage) if (e == v) return true;
        return false;
    }
};

}
