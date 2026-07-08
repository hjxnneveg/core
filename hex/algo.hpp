// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "grid.hpp"

#include <array>
#include <queue>
#include <type_traits>

namespace hjx::hex {

template <unsigned N>
std::array<qrs, N> find_greatest(uint16_t width, auto &&gt) {
    static_assert(N < 1000); // probably

    std::priority_queue<qrs, std::vector<qrs>, std::decay_t<decltype(gt)>>
        heap(std::forward<decltype(gt)>(gt));

    foreach(width, [&](qrs pos) {
        heap.push(pos);
        if (heap.size() > N) heap.pop();
    });

    std::array<qrs, N> ret;

    for (qrs &pos : ret) {
        if (heap.empty()) break;
        pos = heap.top();
        heap.pop();
    }

    return ret;
}


constexpr uint16_t island_flag = 0xffff;

void find_islands(uint16_t width,
                  invocable_r<uint16_t*, qrs> auto &&islenum,
                  std::invocable<qrs, uint16_t> auto &&cb) {
    uint16_t label = 0;
    vector<qrs> stack;
    stack.reserve(std::min(hex::count(width), size_t(1000)));

    auto claim = [&](qrs pos, uint16_t *i) {
        *i = label;
        cb(pos, label);
        stack.push_back(pos);
    };

    auto flood = [&] {
        while (stack.size()) {
            qrs cur = stack.back();
            stack.pop_back();

            foreach_neighbor(cur, width, [&](qrs nb) {
                uint16_t *i = islenum(nb);
                if (i && *i == island_flag) claim(nb, i);
            });
        }
    };

    foreach(width, [&](qrs pos) {
        uint16_t *i = islenum(pos);

        if (i && *i == island_flag) {
            label++;
            claim(pos, i);
            flood();
        }
    });
}

void find_islands(uint16_t width,
                  invocable_r<uint16_t*, qrs> auto &&islenum) {
    find_islands(width,
                 std::forward<decltype(islenum)>(islenum),
                 [](qrs, uint16_t) {});
}

}
