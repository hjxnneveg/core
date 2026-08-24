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

constexpr qrs westunit{-2, 1};
constexpr qrs eastunit{ 2,-1};

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

inline void foreach(uint16_t width, auto &&f) {
    ASSERT_MSG(width & 1, "non-odd width " << width);
    float len = width/2;

    for (float q = -len; q < 0; q++)
        for (float s = len; s >= -len - q; s--)
            f({q, -q-s});

    for (float q = 0; q <= len; q++)
        for (float r = -len; r <= len - q; r++)
            f({q, r});
}

//                   Bands                     //
//                                             //
//                   ------                    //
//                  /      \                   //
//            ------   -4   ------             //
//           /      \      /      \            //
//     ------   -3   ------   -3   ------      //
//    /      \      /      \      /      \     //
//   -   -2   ------   -2   ------   -2   -    //
//    \      /      \      /      \      /     //
//     ------   -1   ------   -1   ------      //
//    /      \      /      \      /      \     //
//   -    0   ------    0   ------    0   -    //
//    \      /      \      /      \      /     //
//     ------    1   ------    1   ------      //
//    /      \      /      \      /      \     //
//   -    2   ------    2   ------    2   -    //
//    \      /      \      /      \      /     //
//     ------    3   ------    3   ------      //
//           \      /      \      /            //
//            ------    4   ------             //
//                  \      /                   //
//                   ------                    //

inline int num_bands(uint16_t width) { return 2 * width - 1; }

// [-width/2..width/2]
inline int band_idx(qrs pos) { return 2 * pos.r() + pos.q(); }

// staggered horizontal bands
// north to south, f(westmost, eastmost, count)
inline void foreach_band(uint16_t width, auto &&f) {
    ASSERT_MSG(width & 1, "non-odd width " << width);
    float len = width/2;

    for (int i = 0; i < len; i++)
        f({-i, i - len}, {i, -len}, i + 1);

    for (int i = 0; i < len; i++) {
        f({-len, i}, {len, i - len}, len + 1);
        f({1 - len, i}, {len - 1, 1 - len + i}, len);
    }

    for (int i = 0; i <= len; i++)
        f({i - len, len}, {len - i, i}, len + 1 - i);
}

namespace detail {

// memory order
constexpr static qrs neighbor_order[] = {qrs::nw_unit(),
                                         qrs::sw_unit(),
                                         qrs::north_unit(),
                                         qrs::south_unit(),
                                         qrs::ne_unit(),
                                         qrs::se_unit()};

}

inline void foreach_neighbor(uint16_t width, qrs pos, auto &&f) {
    ASSERT_MSG(pos.integral(), "non-integral " << pos);

    for (qrs unit : detail::neighbor_order) {
        qrs neighbor = pos + unit;
        if (in_bounds(neighbor, width)) f(neighbor);
    }
}

namespace detail {

class neighbor_iterator {
    qrs center;
    int width, i;

    void set() {
        while (i < 6 && !in_bounds(center + detail::neighbor_order[i], width)) i++;
    }

public:
    using value_type = qrs;
    using difference_type = std::ptrdiff_t;
    using iterator_concept = std::forward_iterator_tag;

    neighbor_iterator(): center(), width(0), i(6) {}

    neighbor_iterator(int width, qrs center, int i): center(center), width(width), i(i) {
        ASSERT_MSG(in_bounds(center, width), center << " vs " << width);
        set();
    }

    qrs operator*() const { ASSERT_LT(i, 6); return center + detail::neighbor_order[i]; }

    neighbor_iterator &operator++() { ASSERT_LT(i, 6); i++; set(); return *this; }
    neighbor_iterator operator++(int) { auto ret = *this; ++*this; return ret; }

    bool operator==(const neighbor_iterator &o) const {
        ASSERT_EQ(center, o.center);
        ASSERT_EQ(width, o.width);
        return i == o.i;
    }
};

struct neighbor_range {
    int width;
    qrs center;
    neighbor_iterator begin() const { return {width, center, 0}; }
    neighbor_iterator end()   const { return {width, center, 6}; }
};

}

inline auto neighbors(int width, qrs pos) {
    ASSERT_MSG(pos.integral(), "non-integral " << pos);
    return detail::neighbor_range{width, pos};
}

// rotate clockwise on even edgelen
constexpr qrs edge_midpoint(direction d, uint16_t width) {
    ASSERT_MSG(width & 1, "non-odd " << width);
    uint16_t maxidx = width / 2;

    switch (d) {
    case EAST: return qrs(  maxidx,           -maxidx / 2     );
    case SE:   return qrs(  maxidx / 2,       (maxidx + 1) / 2);
    case SW:   return qrs(-(maxidx + 1) / 2,   maxidx         );
    case WEST: return qrs( -maxidx,            maxidx / 2     );
    case NW:   return qrs( -maxidx / 2,      -(maxidx + 1) / 2);
    case NE:   return qrs( (maxidx + 1) / 2,  -maxidx         );
    default: ASSERT_MSG(false, "bad dir " << int(d));
    }
}


auto find_greatest(std::same_as<uint16_t> auto width, size_t N, auto &&gt) {
    std::priority_queue<qrs, std::vector<qrs>, std::decay_t<decltype(gt)>> heap(FWD(gt));

    foreach(width, [&](qrs pos) {
        heap.push(pos);
        if (heap.size() > N) heap.pop();
    });

    return heap;
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

            foreach_neighbor(width, cur, [&](qrs nb) {
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
            if (candist >= cur_dist) continue;

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
