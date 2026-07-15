// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "algo.hpp"

#include <core/logging.hpp>
#include <core/mathbits.hpp>
#include <core/reporting.hpp>
#include <core/glyph.hpp>
#include <core/coords.hpp>

#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ostream>
#include <queue>
#include <vector>

namespace hjx::hex {

//           Memory layout (width 5)           //
//                                             //
//                   ------                    //
//                  /      \                   //
//            ------    7   ------             //
//           /      \      /      \            //
//     ------    3   ------   12   ------      //
//    /      \      /      \      /      \     //
//   -    0   ------    8   ------   16   -    //
//    \      /      \      /      \      /     //
//     ------    4   ------   13   ------      //
//    /      \      /      \      /      \     //
//   -    1   ------    9   ------   17   -    //
//    \      /      \      /      \      /     //
//     ------    5   ------   14   ------      //
//    /      \      /      \      /      \     //
//   -    2   ------   10   ------   18   -    //
//    \      /      \      /      \      /     //
//     ------    6   ------   15   ------      //
//           \      /      \      /            //
//            ------   11   ------             //
//                  \      /                   //
//                   ------                    //

// Consider a spiral approach with 0 at the center.
// - allows expanding small maps
// - better cache-locality at center, where the action likely is
//
// e.g. https://github.com/lucidbrot/hexgridspiral

// Map cubics to 0..count-1
constexpr size_t to_scalar(int q, int r, uint16_t width) {
    ASSERT_MSG(in_bounds(q, r, width), "out of bounds " << qrs(q, r));

    size_t trilen = width/2;
    size_t tricnt = trilen * (trilen + 1) / 2;

    size_t full = r + trilen + width * (q + trilen);
    if (q <= 0) return full - tricnt + q * (q + 1) / 2;
    else        return full - tricnt - q * (q - 1) / 2;
}

constexpr qrs scalar_to_qrs(size_t n, uint16_t width) {
    ASSERT_LT(n, count(width));
    const int edgelen = width/2;
    // size of "missing" triangle
    const int trisize = edgelen * (edgelen + 1) / 2;
    //       3
    //    1
    // 0     4
    //    2
    //       5
    int rebased_n = n + trisize;

    if (rebased_n >= width * (width + 1) / 2)
        return -scalar_to_qrs(count(width) - n - 1, width);

    // solve col * (col + 1) / 2 = n
    int rebased_col = sqrtf(0.25f + 2 * rebased_n) - 0.5f;

    float q = rebased_col - 2 * edgelen;
    float r
        = rebased_n
        - rebased_col * (rebased_col + 3) / 2
        + edgelen;

    return {q, r};
}

inline auto neighbors(qrs pos, int width) {
    constexpr static qrs seq[] = {{-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}};

    class iterator {
        qrs center;
        int width, i;

        void set() { while (i < 6 && !in_bounds(center + seq[i], width)) i++; }

    public:
        iterator(qrs center, int width, int i): center(center), width(width), i(i) {
            ASSERT_MSG(in_bounds(center, width), center << " vs " << width);
            set();
        }

        qrs operator*() const { ASSERT_LT(i, 6); return center + seq[i]; }

        iterator &operator++() { ASSERT_LT(i, 6); i++; set(); return *this; }

        bool operator==(const iterator&) const = default;
    };

    struct range {
        qrs center;
        int width;
        iterator begin() { return {center, width, 0}; }
        iterator end()   { return {center, width, 6}; }
    };

    return range{pos, width};
}


template <typename Hex>
class grid {
    const uint16_t width_;
    const size_t count_;
    std::vector<Hex> grid_;

public:
    using hex_type = Hex;

    grid(uint16_t width): width_(width), count_(hex::count(width)), grid_(count_) {
        ASSERT_MSG(width & 1, width);
    }

    grid(grid&&) = delete;
    grid(const grid&) = delete;
    grid &operator=(grid&&) = delete;
    grid &operator=(const grid&) = delete;

    uint16_t width() const { return width_; }
    size_t count() const { return count_; }

    void wipe() { for (Hex &e : grid_) e = Hex(); }

    bool in_bounds(qrs pos) const {
        ASSERT_MSG(pos.integral(), pos << " not integral");
        return hex::in_bounds(pos.q(), pos.r(), width_);
    }

    bool in_bounds(int q, int r) const {
        return in_bounds(q, r, width_);
    }

    void assert_in_bounds(qrs pos) const {
        ASSERT_MSG(in_bounds(pos), pos << " out of bounds");
    }

    bool border(qrs pos) const {
        assert_in_bounds(pos);
        ASSERT_MSG(pos.integral(), pos << " not integral");

        return math::abs(int(pos.q())) == width_/2
            || math::abs(int(pos.r())) == width_/2
            || math::abs(int(pos.s())) == width_/2;
    }

    auto begin() { return grid_.data(); }
    auto end() { return grid_.data() + count_; }

    void foreach(std::invocable<Hex&> auto &&f) {
        for (Hex &hx : grid_) f(hx);
    }

    void foreach(std::invocable<const Hex&> auto &&f) const {
        for (const Hex &hx : grid_) f(hx);
    }

    void foreach(std::invocable<qrs, Hex&> auto &&f) {
        Hex *hx = grid_.data();
        float len = width_/2;

        for (float q = -len; q < 0; q++)
            for (float s = len; s >= -len - q; s--)
                f(qrs{q, -q-s}, *hx++);

        for (float q = 0; q <= len; q++)
            for (float r = -len; r <= len - q; r++)
                f(qrs{q, r}, *hx++);
    }

    void foreach(std::invocable<qrs, Hex&> auto &&f) const {
        const Hex *hx = grid_.data();
        float len = width_/2;

        for (float q = -len; q < 0; q++)
            for (float s = len; s >= -len - q; s--)
                f(qrs{q, -q-s}, *hx++);

        for (float q = 0; q <= len; q++)
            for (float r = -len; r <= len - q; r++)
                f(qrs{q, r}, *hx++);
    }

    void foreach_neighbor(qrs pos, auto &&f) const {
        if (in_bounds(pos.north())) f(pos.north());
        if (in_bounds(pos.ne()))    f(pos.ne());
        if (in_bounds(pos.se()))    f(pos.se());
        if (in_bounds(pos.south())) f(pos.south());
        if (in_bounds(pos.sw()))    f(pos.sw());
        if (in_bounds(pos.nw()))    f(pos.nw());
    }

    auto neighbors(qrs pos) { // fixme - duplication
        constexpr static qrs seq[] = {{-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}};

        class iterator {
            qrs center;
            int width, i;

            void set() { while (i < 6 && !hex::in_bounds(center + seq[i], width)) i++; }

        public:
            iterator(qrs center, int width, int i): center(center), width(width), i(i) {
                ASSERT_MSG(hex::in_bounds(center, width), center << " vs " << width);
                set();
            }

            qrs operator*() const { ASSERT_LT(i, 6); return center + seq[i]; }

            iterator &operator++() { ASSERT_LT(i, 6); i++; set(); return *this; }

            bool operator==(const iterator&) const = default;
        };

        struct range {
            qrs center;
            int width;
            iterator begin() { return {center, width, 0}; }
            iterator end()   { return {center, width, 6}; }
        };

        return range{pos, width()};
    }

    size_t to_scalar(int q, int r) const { return hex::to_scalar(q, r, width_); }

    size_t to_scalar(qrs pos) const {
        assert_in_bounds(pos);
        ASSERT_MSG(pos.integral(), pos);
        return to_scalar(pos.q(), pos.r());
    }

    qrs scalar_to_qrs(size_t n) const { return hex::scalar_to_qrs(n, width_); }

    size_t to_scalar(const Hex &hx) const {
        ASSERT_GE_LT(&hx, &grid_[0], &grid_[0] + count_);
        return &hx - &grid_[0];
    }

    qrs to_qrs(const Hex &hx) const { return scalar_to_qrs(to_scalar(hx)); }

    Hex &to_hex(size_t i) { ASSERT_LT(i, count_); return grid_[i]; }
    Hex &to_hex(qrs pos) { return to_hex(to_scalar(pos)); }
    const Hex &to_hex(size_t i) const { ASSERT_LT(i, count_); return grid_[i]; }
    const Hex &to_hex(qrs pos) const { return to_hex(to_scalar(pos)); }

    const Hex &to_hex(int q, int r) const {
        ASSERT_MSG(in_bounds(q, r), qrs(q, r));
        return to_hex(to_scalar(q, r));
    }

#if 0
    void foreach(rect_t<xy> region, auto &&f) const {
        int min_q = std::floor(qrs{region.nw()}.q());
        int max_q = std::ceil(qrs{region.se()}.q());
        auto min_r = [y=region.nw().y()](float q) { return std::round(y - q * 0.5f); };
        auto max_r = [y=region.se().y()](float q) { return std::round(y - q * 0.5f); };

        for (int q = min_q; q <= max_q; q++)
            for (int r = min_r(q); r <= max_r(q); r++)
                f(q, r);
    }
#endif

    static void serialize(archive &a, const grid<Hex> &map) {
        glyph_t g = glyph::worldmap(map.width());
        a << g;
        map.foreach([&](const Hex &hx) { a << hx; });
    }

    friend std::ostream &operator<<(std::ostream &os, const grid<Hex> &g) {
        return os << "[hexgrid " << type2str<Hex>() << " " << g.width() << "]";
    }
};

}
