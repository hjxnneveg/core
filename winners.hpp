// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#pragma once

#include "reporting.hpp"

#include <utility>

namespace hjx {

// result depends on order of appraisal--might skip the max
// assumes T{} is false
template <typename T, unsigned N, typename F>
class winners {
    F conflict;
    T arr[N]{}; // unsorted

public:
    explicit winners(F conflict): conflict(std::move(conflict)) { ASSERT(!T{}); }

    void appraise(const T &candidate) {
        T *enemy = nullptr;
        T *caboose = arr;

        for (T &member : arr) {
            if (!member) {
                caboose = &member;
                break;
            }

            if (conflict(member, candidate)) {
                if (enemy) return; // two conflicts, SOL
                enemy = &member;
            }

            if (member.score < caboose->score) caboose = &member;
        }

        if (enemy) {
            if (enemy->score < candidate.score) *enemy = candidate;
            return;
        }

        if (!*caboose || caboose->score < candidate.score) *caboose = candidate;
    }

    const T &operator[](unsigned i) const { ASSERT_LT(i, N); return arr[i]; }

    const T *begin() const { return arr; }
    const T *end() const { return arr + N; }
};

// for type deduction of conflict
template <typename T, unsigned N, typename F>
auto make_winners(F &&conflict) {
    return winners<T, N, std::decay_t<F>>(std::forward<F>(conflict));
}

}
