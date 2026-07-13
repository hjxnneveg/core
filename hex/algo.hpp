// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include <core/typeutils.hpp>
#include <core/coords.hpp>

#include <array>
#include <queue>
#include <type_traits>
#include <vector>

namespace hjx::hex {

constexpr size_t count(uint16_t width) {
    return size_t(width) * width - width/2 * (width/2 + 1);
}

constexpr bool in_bounds(int q, int r, uint16_t width) {
    float len = width/2;
    return math::abs(q) <= len && math::abs(r) <= len && math::abs(-q-r) <= len;
}

constexpr bool in_bounds(qrs pos, uint16_t width) {
    if (!pos) return false;
    ASSERT_MSG(pos.integral(), "non-integral " << pos);
    return in_bounds(pos.q(), pos.r(), width);
}


template <unsigned N>
std::array<qrs, N> find_greatest(uint16_t width, auto &&gt) {
    static_assert(N < 1000); // probably

    std::priority_queue<qrs, std::vector<qrs>, std::decay_t<decltype(gt)>> heap(FWD(gt));

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
    std::vector<qrs> stack;
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

void find_islands(uint16_t width, invocable_r<uint16_t*, qrs> auto &&islenum) {
    find_islands(width, FWD(islenum), [](qrs, uint16_t) {});
}


// `from` and `to` are not symmetric.  The beginning of the path from `from` always
// heads toward `to`, but the end of the path doesn't necessarily come from `from`'s
// direction.  Won't backtrack--if it gets itself into a corner it just gives up and
// returns false.
bool tunnel(auto &&rand, qrs from, qrs to, int width,
            std::invocable<qrs> auto &&cb,
            invocable_r<bool, qrs> auto &&ok) {
    ASSERT_MSG(from.integral(), "non-integral " << from);
    ASSERT_MSG(to.integral(), "non-integral " << to);

    for (;;) {
        bool progress = false;
        float cur_dist = square_dist(from, to);

        for (unsigned i : permutation<6>(rand())) {
            qrs candidate = from + qrs{direction(i)};

            if (candidate == to) return true;
            if (!in_bounds(candidate, width)) continue;
            if (!ok(candidate)) continue;

            if (square_dist(candidate, to) < cur_dist) {
                from = candidate;
                progress = true;
                cb(from);
                break;
            }
        }

        if (!progress) return false;
    }
}

bool tunnel(auto &&rand, qrs from, qrs to, int width, std::invocable<qrs> auto &&cb) {
    return tunnel(FWD(rand), from, to, width, FWD(cb), [](qrs) { return true; });
}

bool wendy_tunnel(auto &&rand, qrs from, qrs to, int width,
                  std::invocable<qrs> auto &&cb,
                  invocable_r<bool, qrs> auto &&ok) {
    ASSERT_MSG(from.integral(), "non-integral " << from);
    ASSERT_MSG(to.integral(), "non-integral " << to);

    for (;;) {
        bool progress = false;
        float cur_dist = square_dist(from, to);
        qrs frontrunner;
        float best = inf;

        for (unsigned i : permutation<6>(rand())) {
            qrs candidate = from + qrs{direction(i)};

            if (candidate == to) return true;
            if (!in_bounds(candidate, width)) continue;
            if (!ok(candidate)) continue;

            float candist = square_dist(candidate, to);

            if (candist < cur_dist) {
                if (candist < best) {
                    frontrunner = candidate;
                    best = candist;
                    continue;
                }

                from = candidate;
                progress = true;
                cb(from);
                break;
            }
        }

        if (!progress) {
            if (frontrunner) {
                from = frontrunner;
                cb(from);
                continue;
            }

            return false;
        }
    }
}

bool wendy_tunnel(auto &&rand, qrs from, qrs to, int width, std::invocable<qrs> auto &&cb) {
    return wendy_tunnel(FWD(rand), from, to, width, FWD(cb), [](qrs) { return true; });
}

}
