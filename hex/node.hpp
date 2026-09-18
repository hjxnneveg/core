// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include <core/mathbits.hpp>
#include <core/reporting.hpp>

#include <cstdint>
#include <ostream>

// Intra-hex node coordinates.
//
// Node coords are at 6× qrs resolution.  Hex centers are multiple of 6 (two nodes
// sit in the same slot of their hex if they agree mod 6).
//
// Valid node positions are (q - r) % 3 == 0.  A hex owns 12 nodes--its 7 interior
// nodes and 5 of the 12 on its perimeter.

//            ♢       ♢       ♢           //
//                                        //
//                                        //
//                                        //
//        ♦       ♦       ♦       ♢       //
//      -3,+0   -1,-1   +1,-2             //
//                                        //
//                                        //
//    ♦       ♦       ♦       ♦       ♢   //
//  -4,+2   -2,+1   +0,+0   +2,-1         //
//                                        //
//                                        //
//        ♦       ♦       ♦       ♢       //
//      -3,+3   -1,+2   +1,+1             //
//                                        //
//                                        //
//            ♦       ♦       ♢           //
//          -2,+4   +0,+3                 //

#define ARCHNODE_CORE_TRAITS_SEP(X, SEP)        \
    /* i   q   r */                             \
    X( 0,  0,  0) SEP                           \
    X( 1,  1, -2) SEP                           \
    X( 2,  1,  1) SEP                           \
    X( 3,  2, -1) SEP                           \
    X( 4,  0,  3) SEP                           \
    X( 5, -3,  0) SEP                           \
    X( 6, -4,  2) SEP                           \
    X( 7, -2,  1) SEP                           \
    X( 8, -1, -1) SEP                           \
    X( 9, -3,  3) SEP                           \
    X(10, -1,  2) SEP                           \
    X(11, -2,  4)

#define ARCHNODE_CORE_TRAITS(X) ARCHNODE_CORE_TRAITS_SEP(X,)


namespace hjx::hex::impl {

constexpr uint64_t sparse_nsbit(int q, int r) {
    return uint64_t(1) << ((q + 4) * 8 + r + 2);
}

constexpr uint64_t NODESHIFT_MASK = []{
    uint64_t u = 0;
#define X(i, q, r) u |= sparse_nsbit(q, r);
    ARCHNODE_CORE_TRAITS(X);
#undef X
    return u;
}();

// Nibble j holds the node's offset from its center, biased nonnegative:
// dq [-4,2] stored as dq+4, dr [-2,4] as dr+2.
constexpr uint64_t DQ_BY_J = 0x6352'4130'2514;
constexpr uint64_t DR_BY_J = 0x1106'5544'3322;

constexpr int nib(uint64_t packed, int j) { return int((packed >> (j * 4)) & 0xf); }

// (r mod 6) * 2 + parity((q - r) / 3)
// accepts any points on the grid
constexpr int coord_index(int16_t q, int16_t r) {
    ASSERT_MSG((q - r) % 3 == 0, "off-grid node (" << q << "," << r << ")");
    // bump r by a big multiple of 6 to make it non-negative, so we can do unsigned `%`
    return int(unsigned(r + 60000) % 6u) * 2 + ((q ^ r) & 1);
}

constexpr bool is_nodeshift(int q, int r) {
    if (q < -4 || q > 2 || r < -2 || r > 4) return false;
    return impl::NODESHIFT_MASK & impl::sparse_nsbit(q, r);
}

// minimal perfect hash [0..11]
constexpr size_t shift_index(int8_t q, int8_t r) {
    ASSERT_MSG(is_nodeshift(q, r), "bad nodeshift (" << +q << "," << +r << ")");
    uint64_t bits = uint8_t(q) << 10 | uint8_t(r);
    bits *= 0xb617'90f9'78d2'8d2d;
    return bits >> 59;
}

}


namespace hjx::hex {

using impl::is_nodeshift;

class nodecoords {
    using index_t = int16_t;
    constexpr static index_t nilrepr = std::numeric_limits<index_t>::min();

    index_t q_;
    index_t r_;

    constexpr bool is_center() const { return q_ % 6 == 0 && r_ % 6 == 0; }

    constexpr static int separation(int aq, int ar, int bq, int br) {
        int as = -aq - ar;
        int bs = -bq - br;
        return math::abs(aq - bq) + math::abs(ar - br) + math::abs(as - bs);
    }

public:
    constexpr nodecoords(): q_(nilrepr), r_(nilrepr) {}

    constexpr nodecoords(int q, int r): q_(q), r_(r) {
        ASSERT_FITS(q, index_t);
        ASSERT_FITS(r, index_t);
        ASSERT_FITS(-q - r, index_t);

        ASSERT_NE(q, nilrepr);
        ASSERT_NE(r, nilrepr);
        ASSERT_NE(-q - r, nilrepr);

        ASSERT_MSG((q - r) % 3 == 0, "off-grid node (" << q << "," << r << ")");
    }

    constexpr explicit operator bool() const { return q_ != nilrepr; }

    constexpr int16_t q() const { return q_; }
    constexpr int16_t r() const { return r_; }
    constexpr int16_t s() const { return -q() - r(); }

    nodecoords shifted(int q, int r) const { return {q_ + q, r_ + r}; }

    nodecoords operator+(nodecoords c) const { return {q_ + c.q_, r_ + c.r_}; }
    nodecoords operator-(nodecoords c) const { return {q_ - c.q_, r_ - c.r_}; }
    nodecoords operator-() const { return {-q_, -r_}; }

    bool operator==(const nodecoords&) const = default;
    auto operator<=>(const nodecoords&) const = default;

    constexpr bool owns(nodecoords c) const {
        ASSERT(is_center());

        // (-1, 1) is the center of the owned nodes
        // ( 0, 3) is an arbitrary owned node at greatest distance

        //            ♢       ♢       ♢            //
        //                                         //
        //                                         //
        //                                         //
        //        ♦       ♦       ♦       ♢        //
        //                                         //
        //                                         //
        //                   0,0                   //
        //    ♦       ♦       ♦       ♦       ♢    //
        //              -1,1                       //
        //                °                        //
        //                                         //
        //        ♦       ♦       ♦       ♢        //
        //                                         //
        //                                         //
        //                   0,3                   //
        //            ♦       ♦       ♢            //

        constexpr int max = separation(-1, 1, 0, 3);
        return separation(q_ - 1, r_ + 1, c.q_, c.r_) <= max;
    }

    // center node of the hex owning this node
    constexpr nodecoords chief() const {
        using namespace impl;

        size_t j = coord_index(q_, r_);
        return nodecoords(q_ - (nib(DQ_BY_J, j) - 4),
                          r_ - (nib(DR_BY_J, j) - 2));
    }

    friend std::ostream &operator<<(std::ostream &os, nodecoords nc) {
        return os << "(" << nc.q_ << "/6, " << nc.r_ << "/6)";
    }
};


void foreach_nodeshift(std::invocable<nodecoords> auto &&f) {
#define X(i, q, r) f(nodecoords{q, r});
    ARCHNODE_CORE_TRAITS(X);
#undef X
}

void foreach_nodeshift(std::invocable<int, nodecoords> auto &&f) {
#define X(i, q, r) f(i, nodecoords{q, r});
    ARCHNODE_CORE_TRAITS(X);
#undef X
}

// Travel along the edges of the 3-6 kisrhombille.
// Two single-unit "moves" land you on a node.
// fore + starboard must be even.
constexpr nodecoords bearing(int ock, int fore, int starboard) {
    ASSERT_NOT((fore + starboard) & 1);

    if (ock >= 6) ock -= 6, fore = -fore, starboard = -starboard;

    auto f = [](int heavy, int light) { return (heavy * 3 + light) / 2; };

    switch (ock) {
    case 0: return {starboard,           -f(fore, starboard)};
    case 1: return {f(starboard, fore),  -fore};
    case 2: return {f(fore, starboard),  f(-fore, starboard)};
    case 3: return {fore,                f(starboard, -fore)};
    case 4: return {f(fore, -starboard), starboard};
    case 5: return {f(-starboard, fore), f(starboard, fore)};
    default: ERROR(ock << " o'clock");
    }
}

constexpr int16_t reach(int ock, nodecoords nc) {
    auto f = [=](int ock)->int16_t {
        switch (ock % 6) {
        case 0: return -(nc.q() + 2 * nc.r()) / 3;
        case 1: return -nc.r();
        case 2: return (nc.q() - nc.r()) / 3;
        case 3: return nc.q();
        case 4: return (2 * nc.q() + nc.r()) / 3;
        case 5: return -nc.s();
        default: ERROR(ock << " o'clock");
        }
    };

    return ock < 6 ? f(ock) : -f(ock);
}


/// static_assert battery.  All compile-time below.

namespace impl {

constexpr nodecoords nodeshifts_[12] = {
#define X(i, q, r) {q, r},
    ARCHNODE_CORE_TRAITS(X)
#undef X
};


consteval bool constants_match_offsets() {
    uint32_t seen = 0;

    for (int k = 0; k < 12; k++) {
        int dq = nodeshifts_[k].q();
        int dr = nodeshifts_[k].r();
        size_t j = coord_index(dq, dr);

        if (j > 11) return false;
        if (seen & (1u << j)) return false;

        seen |= 1u << j;

        if (nib(DQ_BY_J, j) - 4 != dq) return false;
        if (nib(DR_BY_J, j) - 2 != dr) return false;
    }

    return seen == 0xfff;
}

static_assert(constants_match_offsets());


// nodeshifts survive nibble bias
consteval bool offsets_fit_nibbles() {
    for (int k = 0; k < 12; k++) {
        if (nodeshifts_[k].q() + 4 < 0 || nodeshifts_[k].q() + 4 > 15) return false;
        if (nodeshifts_[k].r() + 2 < 0 || nodeshifts_[k].r() + 2 > 15) return false;
    }

    return true;
}

static_assert(offsets_fit_nibbles());


// every owned node resolves to the hex's center
consteval bool correct_ownership() {
    for (int q = -4; q <= 4; q++)
        for (int r = -4; r <= 4; r++)
            for (int k = 0; k < 12; k++) {
                nodecoords node{6 * q + nodeshifts_[k].q(),
                                6 * r + nodeshifts_[k].r()};
                nodecoords chief = node.chief();

                if (chief.q() != 6 * q || chief.r() != 6 * r) return false;
            }

    return true;
}

static_assert(correct_ownership());

}

}
