// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "reporting.hpp"

#include <initializer_list>
#include <type_traits>

namespace hjx {

// "enum" shortlist value type.  T() is considered "none".
template <typename T, unsigned N>
requires std::is_nothrow_swappable_v<T>
class eslist {
    static_assert(N <= 4);
    T a[N] = {}; // sorted

public:
    eslist() {}

    eslist(std::initializer_list<T> l) {
        ASSERT_LE(l.size(), N);
        for (T e : l) add(e);
    }

    void add(T e) {
        for (unsigned i = 0; i < N; i++) {
            if (!a[i]) { a[i] = e; return; }
            if (e < a[i]) std::swap(e, a[i]);
        }

        ERROR("full " << *this << " \\ " << e);
    }

    size_t size() const {
        if (N > 3 && a[3]) return 4;
        if (N > 2 && a[2]) return 3;
        if (N > 1 && a[1]) return 2;
        if (N > 0 && a[0]) return 1;
        return 0;
    }

    T &operator[](int i) { ASSERT_GE_LT(i, 0, N); return a[i]; }

    T *begin() { return a; }
    T *end() { return a + size(); }

    const T *begin() const { return a; }
    const T *end() const { return a + size(); }

    friend std::ostream &operator<<(std::ostream &os, const eslist<T, N> &l) {
        os << '[';

        for (unsigned i = 0; i < N && l.a[i]; i++) {
            if (i) os << ' ';
            os << l.a[i];
        }

        return os << ']';
    }
};

}
