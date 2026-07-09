// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "honest_float.hpp"
#include "direction.hpp"
#include "mathbits.hpp"
#include "reporting.hpp"
#include "entropy.hpp"
#include "angle.hpp"
#include "archive.hpp"

#include <cmath>
#include <cstring>
#include <compare>
#include <iomanip>
#include <limits>
#include <ostream>

namespace hjx {

struct coords {
    float phi, psi;

    constexpr coords(): phi(nan::make()), psi(nan::make()) {}

    constexpr coords(float phi, float psi):
        phi(phi + 0), psi(psi + 0) { sanity_check(); }

    constexpr void sanity_check() const {
        if (std::isnan(phi) || std::isnan(psi)) {
            ASSERT_EQ(std::bit_cast<uint32_t>(phi), nan::quiet_bits);
            ASSERT_EQ(std::bit_cast<uint32_t>(psi), nan::quiet_bits);
            return;
        }

        ASSERT_NE(std::bit_cast<uint32_t>(phi), std::bit_cast<uint32_t>(-0.f));
        ASSERT_NE(std::bit_cast<uint32_t>(psi), std::bit_cast<uint32_t>(-0.f));
    }

    constexpr bool nilp() const { return std::bit_cast<uint32_t>(phi) == nan::quiet_bits; }

    constexpr unsigned nilv() const {
        ASSERT_MSG(nilp(), "non-nil " << *this);
        return nan::payload(psi);
    }

    constexpr explicit operator bool() const { return !nilp(); }

    bool integral() const { return phi == std::trunc(phi) && psi == std::trunc(psi); }

    friend std::ostream &operator<<(std::ostream &os, coords p) {
        return os << "(" << honest_float(p.phi) << "," << honest_float(p.psi) << ")";
    }

protected:
    static constexpr bool untyped_eq(coords a, coords b) {
        return !std::memcmp(&a, &b, sizeof(coords));
    }
};

struct polar;
struct qrs;

constexpr void qrs2xy(float *x, float *y, const qrs &c);

struct xy : coords {
    constexpr xy(): coords() {}

    constexpr xy(float x, float y): coords(x, y) {}

    constexpr explicit xy(const qrs &c) { qrs2xy(&phi, &psi, c); }

    explicit xy(polar);

    static xy from(auto &&whatever) {
        auto [x, y] = whatever;
        return {float(x), float(y)};
    }

    constexpr float &x() { return phi; }
    constexpr float &y() { return psi; }
    constexpr const float &x() const { return phi; }
    constexpr const float &y() const { return psi; }

    constexpr bool operator==(const xy &o) const { return untyped_eq(*this, o); }

    uint64_t id() const {
        ASSERT_MSG(integral(), *this << " not integral");
        return concat(std::bit_cast<uint32_t>(x()), std::bit_cast<uint32_t>(y()));
    }

    constexpr float abs() const { return std::hypot(x(), y()); }

    constexpr float radians() const { return std::atan2(y(), x()); }

    constexpr xy normalize() const {
        ASSERT_MSG(abs() > 0 && std::isnormal(abs()), *this);
        return *this * (1.f / abs());
    }

    constexpr float dot(const xy &o) const { return x() * o.x() + y() * o.y(); }

    constexpr float autodot() const { return x() * x() + y() * y(); }

    constexpr uint64_t hash() const { return ::hjx::hash(id()); }

    constexpr xy operator+(const xy &o) const { return {x() + o.x(), y() + o.y()}; }

    constexpr xy operator-(const xy &o) const { return {x() - o.x(), y() - o.y()}; }

    constexpr xy operator-() const { return {-x(), -y()}; }

    constexpr xy operator*(const auto &v) const { return {x() * v, y() * v}; }

    constexpr xy operator/(const auto &v) const {
        ASSERT(v);
        return xy(x() / v, y() / v);
    }

    constexpr xy &operator+=(const xy &o) { return *this = *this + o; }
    constexpr xy &operator*=(const auto &o) { return *this = *this * o; }

    xy &operator/=(const auto &o) { return *this = *this / o; }

    friend std::ostream &operator<<(std::ostream &o, const xy &c) {
        return c ?
            o << "(" << honest_float(c.x()) << "," << honest_float(c.y()) << ")" :
            o << "(nil)";
    }
};

constexpr xy operator*(const auto &val, xy c) { return c * val; }

struct polar {
    float r;
    angle theta;

    polar(float r, angle theta): r(r), theta(theta) { ASSERT_GE(r, 0); }

    explicit polar(xy c): r(c.abs()), theta(angle::radians(std::atan2(c.y(), c.x()))) {}
};

inline std::ostream &operator<<(std::ostream &o, const polar &c) {
    return o << '(' << honest_float(c.r) << ',' << c.theta << ')';
}

inline xy::xy(polar c): coords(c.r * std::cos(c.theta.rad()),
                               c.r * std::sin(c.theta.rad())) {}

constexpr float square_dist(qrs, qrs);

struct qrs : coords {
    constexpr qrs(): coords() {}

    constexpr qrs(float q, float r): coords(q, r) {}

    constexpr qrs(float q, float r, float IF_ASSERTS_ON(s)): coords(q, r) {
        ASSERT_EQ(s, -q - r);
    }

    constexpr explicit qrs(xy c): coords() {
        phi = c.x() * (2 / math::sqrt3);
        psi = c.y() - phi / 2;
        sanity_check();
    }

    constexpr explicit qrs(direction d) {
        switch (d) {
        case direction::none:  *this = qrs{0, 0};    break;
        case direction::north: *this = north_unit(); break;
        case direction::ne:    *this = ne_unit();    break;
        case direction::se:    *this = se_unit();    break;
        case direction::south: *this = south_unit(); break;
        case direction::sw:    *this = sw_unit();    break;
        case direction::nw:    *this = nw_unit();    break;
        default: ERROR("bad direction " << int(d));
        }
    }

    constexpr qrs north() const { return *this + north_unit(); }
    constexpr qrs ne()    const { return *this + ne_unit(); }
    constexpr qrs se()    const { return *this + se_unit(); }
    constexpr qrs south() const { return *this + south_unit(); }
    constexpr qrs sw()    const { return *this + sw_unit(); }
    constexpr qrs nw()    const { return *this + nw_unit(); }

    constexpr static qrs north_unit() { return qrs( 0, -1); }
    constexpr static qrs ne_unit()    { return qrs( 1, -1); }
    constexpr static qrs se_unit()    { return qrs( 1,  0); }
    constexpr static qrs south_unit() { return qrs( 0,  1); }
    constexpr static qrs sw_unit()    { return qrs(-1,  1); }
    constexpr static qrs nw_unit()    { return qrs(-1,  0); }

    constexpr static qrs ne_vertex() { return qrs( 1/3.f, -2/3.f); }
    constexpr static qrs  e_vertex() { return qrs( 2/3.f, -1/3.f); }
    constexpr static qrs se_vertex() { return qrs( 1/3.f,  1/3.f); }
    constexpr static qrs sw_vertex() { return qrs(-1/3.f,  2/3.f); }
    constexpr static qrs  w_vertex() { return qrs(-2/3.f,  1/3.f); }
    constexpr static qrs nw_vertex() { return qrs(-1/3.f, -1/3.f); }

    constexpr static qrs  n_edge() { return north_unit() / 2; }
    constexpr static qrs ne_edge() { return ne_unit() / 2; }
    constexpr static qrs se_edge() { return se_unit() / 2; }
    constexpr static qrs  s_edge() { return south_unit() / 2; }
    constexpr static qrs sw_edge() { return sw_unit() / 2; }
    constexpr static qrs nw_edge() { return nw_unit() / 2; }

    constexpr bool operator==(const qrs &o) const { return untyped_eq(*this, o); }

    constexpr std::partial_ordering operator<=>(const qrs &o) const {
        if (!*this && !o) return std::partial_ordering::equivalent;
        if (!*this || !o) return std::partial_ordering::unordered;

        // memory order
        if (q() == o.q()) return r() <=> o.r();
        return q() <=> o.q();
    }

    constexpr float q() const { return phi; }
    constexpr float r() const { return psi; }
    constexpr float s() const { return -q() - r() + 0; }

    constexpr qrs operator+(const qrs &o) const {
        ASSERT(*this); ASSERT(o);
        return qrs(q() + o.q(), r() + o.r());
    }

    constexpr qrs operator-(const qrs &o) const {
        ASSERT(*this); ASSERT(o);
        return qrs(q() - o.q(), r() - o.r());
    }

    constexpr qrs operator-() const {
        ASSERT(*this);
        return qrs(-q(), -r());
    }

    constexpr qrs &operator+=(const qrs &o) {
        return *this = *this + o;
    }

    constexpr qrs &operator-=(const qrs &o) {
        return *this = *this - o;
    }

    constexpr qrs operator/(const auto &val) const {
        ASSERT(*this);
        ASSERT(val);
        return qrs(q() / val, r() / val);
    }

    uint64_t id() const {
        ASSERT_MSG(integral(), *this << " not integral");
        return concat(int32_t(q()), int32_t(r()));
    }

    uint64_t hash() const { return ::hjx::hash(id()); }

    // cart_dist doesn't guarantee flint on axes
    float abs() const { return sqrt(sqabs(q(), r())); }

    constexpr static float sqabs(float q, float r) {
        return (q * q + r * r + (q + r) * (q + r)) * 0.5f;
    }

    float manhattan() const {
        return (std::abs(q()) + std::abs(r()) + std::abs(s())) * 0.5f;
    }

    float x() const { ASSERT(*this); return math::sqrt3/2 * q(); }
    float y() const { ASSERT(*this); return r() + 0.5f * q(); }

    xy cartesian() const { return xy{x(), y()}; }

    constexpr float radians() const { return xy{*this}.radians(); }

    constexpr qrs rotate_l(unsigned n=1) const {
        ASSERT(*this);
        ASSERT_GE_LE(n, 0, 5);
        switch (n) {
        case 0: return *this;
        case 1: return qrs{-s(), -q()};
        case 2: return qrs{ r(),  s()};
        case 3: return qrs{-q(), -r()};
        case 4: return qrs{ s(),  q()};
        case 5: return qrs{-r(), -s()};
        default: ERROR(n);
        }
    }

    constexpr qrs rotate_r(unsigned n=1) const { return rotate_l(6 - n); }

    qrs snap() const {
        const qrs candidates[4] = {
            qrs(ceilf (q()), ceilf (r())),
            qrs(ceilf (q()), floorf(r())),
            qrs(floorf(q()), ceilf (r())),
            qrs(floorf(q()), floorf(r()))
        };

        const qrs *winner = &candidates[0];
        float min_dist = square_dist(*this, *winner);
        for (size_t i = 1; i < 4; i++) {
            float dist = square_dist(*this, candidates[i]);
            if (dist < min_dist) {
                min_dist = dist;
                winner = &candidates[i];
            }
        }

        return *winner;
    }

    static void serialize(archive &a, const qrs &c) {
        a.put(c.q());
        a.put(c.r());
    }

    static qrs deserialize(auto &x) {
        float q = x.get_float();
        float r = x.get_float();
        return {q, r};
    }

    friend std::ostream &operator<<(std::ostream &os, qrs c) {
        if (!c) return os << "[dud]";
        return os << '[' << honest_float(c.q())
                  << ',' << honest_float(c.r())
                  << ',' << honest_float(c.s()) << ']';
    }
};

static_assert(sizeof(qrs) <= sizeof(size_t));

constexpr void qrs2xy(float *x, float *y, const qrs &c) {
    *x = math::sqrt3/2 * c.q();
    *y = c.r() + 0.5f * c.q();
}

template <typename T>
constexpr qrs operator*(qrs c, const T &val) {
    ASSERT(c);
    return qrs(c.q() * val, c.r() * val);
}

template <typename T>
constexpr qrs operator*(const T &val, qrs c) { return c * val; }

}

template<>
struct std::hash<hjx::qrs> {
    size_t operator()(const hjx::qrs &c) const noexcept { return c.hash(); }
};

namespace hjx {

constexpr float square_dist(qrs a, qrs b) {
    float q = a.q() - b.q();
    float r = a.r() - b.r();
    float s = a.s() - b.s();
    return (q * q + r * r + s * s) * 0.5f;
}

constexpr float square_dist(xy a, xy b) {
    float dx = b.x() - a.x();
    float dy = b.y() - a.y();
    return dx * dx + dy * dy;
}

inline float cart_dist(xy a, xy b) {
    return std::hypot(b.x() - a.x(), b.y() - a.y());
}

inline float cart_dist(qrs a, qrs b) {
    return cart_dist(a.cartesian(), b.cartesian());
}

constexpr inline float cart_dist_constexpr(xy a, xy b) {
    return std::sqrt(square_dist(a, b));
}

inline float manhattan_dist(qrs a, qrs b) {
    ASSERT(a.integral());
    ASSERT(b.integral());
    using std::abs;
    return (abs(a.q() - b.q()) + abs(a.r() - b.r()) + abs(a.s() - b.s())) / 2;
}

inline qrs to_qrsi(float x, float y) {
    constexpr float qscale = 2 / math::sqrt3; // 1 / (1.5 * edgelen)

    float qf = x * qscale;
    float rf = y - 0.5f * qf;
    float sf = -qf - rf;

    float q = std::round(qf), r = std::round(rf), s = std::round(sf);
    float dq = std::abs(q - qf), dr = std::abs(r - rf), ds = std::abs(s - sf);

    if      (dq > dr && dq > ds) q = -r - s;
    else if (dr > ds)            r = -q - s;
    // ds largest: q, r already correct

    return {q, r};
}

}
