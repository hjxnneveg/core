// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "honest_float.hpp"
#include "reporting.hpp"
#include "typeutils.hpp"
#include "wrappers.hpp"

#include <bit>
#include <concepts>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <numbers>
#include <numeric>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

namespace hjx {

template <typename T>
inline constexpr size_t bitlength = 8 * sizeof(T);

static_assert(std::endian::native == std::endian::big ||
              std::endian::native == std::endian::little);

constexpr bool little_endian() noexcept {
    return std::endian::native == std::endian::little;
}

constexpr uint64_t uint64max = 0xffff'ffff'ffff'ffff;
constexpr  int64_t  int64max = 0x7fff'ffff'ffff'ffff;
constexpr  int64_t  int64min = 0x8000'0000'0000'0000;

constexpr uint32_t uint32max = 0xffff'ffff;
constexpr  int32_t  int32max = 0x7fff'ffff;
constexpr  int32_t  int32min = 0x8000'0000;

constexpr double inf = std::numeric_limits<double>::infinity();

template<trivially_copyable T>
using phony_uint = std::conditional_t<sizeof(T) == 1, uint8_t,
                   std::conditional_t<sizeof(T) == 2, uint16_t,
                   std::conditional_t<sizeof(T) == 4, uint32_t,
                   std::conditional_t<sizeof(T) == 8, uint64_t, void>>>>;

template<trivially_copyable T>
constexpr phony_uint<T> to_uint(const T &v) noexcept {
    return std::bit_cast<phony_uint<T>>(v);
}

template <std::integral T>
T byteswap(T v) noexcept {
    for (unsigned i = 0; i < sizeof(T) / 2; i++) {
        uint8_t *p = reinterpret_cast<uint8_t*>(&v);
        std::swap(*(p + i), *(p + sizeof(T) - 1 - i));
    }

    return v;
}

template <std::integral T>
T little_endianize(T v) noexcept { return little_endian() ? v : byteswap(v); }

template <std::integral T>
T big_endianize(T v) noexcept { return little_endian() ? byteswap(v) : v; }

template <typename T>
constexpr phony_uint<T> leftmost_bit =
    std::bit_floor(std::numeric_limits<phony_uint<T>>::max());


template <std::integral ...Ts>
constexpr uint64_t concat(Ts ...vs) {
    static_assert((sizeof vs + ...) <= 8);

    if constexpr (sizeof...(Ts) == 1) return to_uint((vs, ...));
    else {
        uint64_t ret = 0;
        auto append = [&](auto v) { ret <<= sizeof(v) * 8; ret |= to_uint(v); };
        (append(vs), ...);
        return ret;
    }
}

template <std::integral... Ts>
constexpr void unconcat(uint64_t n, int skip, Ts *...ps) {
    static_assert((sizeof *ps + ...) <= 8);
    ASSERT_GE_LE(skip, 0, 7);
    ASSERT_LE((sizeof *ps + ...), 8 - skip);

    n >>= 8 * skip;

    if constexpr (sizeof...(Ts) == 1) *(ps, ...) = n;
    else {
        auto append = [&](auto *p) { *p = n; n >>= sizeof(*p) * 8; };
        (append(ps), ...);
    }
}

template <std::integral... Ts>
constexpr void unconcat(uint64_t n, Ts *...ps) { unconcat(n, 0, ps...); }


template<std::floating_point T>
inline std::string frepr(T x, bool bits=false) {
    using repr_t = phony_uint<T>;
    constexpr int numbits = sizeof x * 8;
    constexpr int manbits = std::numeric_limits<T>::digits - 2;

    std::ostringstream ss;
    repr_t rep = std::bit_cast<repr_t>(x);
    ss << x << " [" << hexos() << rep;

    if (bits) {
        for (int i = numbits - 1; i >= 0; i--) {
            if (i >= numbits - 2 || i == manbits) ss << " ";
            ss << ((rep & (repr_t(1) << i)) ? "●" : "○");
        }
    }

    ss << "]";
    return ss.str();
}

constexpr uint32_t fbits(float x) { return std::bit_cast<uint32_t>(x); }


namespace nan {

constexpr uint32_t quiet_bits = 0x7fc0'0000;
constexpr uint32_t payload_mask = 0x003f'ffff;

static_assert(std::numeric_limits<float>::has_quiet_NaN);
static_assert(std::bit_cast<uint32_t>(std::numeric_limits<float>::quiet_NaN())
              == quiet_bits);

constexpr float make(unsigned payload=0) NOTHROW {
    ASSERT_LE(payload, payload_mask);
    return std::bit_cast<float>(uint32_t(quiet_bits | payload));
}

constexpr unsigned payload(float nan) NOTHROW {
    ASSERT_MSG(std::isnan(nan), frepr(nan));
    return std::bit_cast<uint32_t>(nan) & payload_mask;
}

}


namespace math {

struct interval {
    float min;
    float max;

    interval(float min, float max) noexcept: min(min), max(max) {}

    float length() const noexcept { return max - min; }
    bool proper() const noexcept { return max >= min; }
    interval flip() const noexcept { return interval{max, min}; }

    friend ostream &operator<<(ostream &os, interval v) {
        return os << "[" << honest_float(v.min) << ", " << honest_float(v.max) << "]";
    }
};

class affine_map {
    interval from;
    interval to;
    float mid;
    float scale;

public:
    affine_map(interval from, interval to) noexcept:
        from(from.proper() ? from : from.flip()),
        to(from.proper() ? to : to.flip()),
        mid(std::midpoint(from.min, from.max)),
        scale((to.max - to.min) / (from.max - from.min)) {}

    bool valid() const noexcept {
        return std::isfinite(scale) && mid != from.min && mid != from.max;
    }

    float operator()(float x) const NOTHROW {
        ASSERT_MSG(valid(), "invalid " << *this);
        float af = x >= mid ? from.max : from.min;
        float at = x >= mid ? to.max : to.min;
        return std::fma(x - af, scale, at);
    }

    friend ostream &operator<<(ostream &os, const affine_map &map) {
        os << map.from << "↦" << map.to;
        if (!map.valid())
            os << " mid: " << honest_float(map.mid)
               << " scale: " << honest_float(map.scale);
        return os;
    }
};

constexpr float sqrt2 = std::numbers::sqrt2;
constexpr float sqrt3 = std::numbers::sqrt3;
constexpr float pi = std::numbers::pi;

constexpr auto abs(std::signed_integral auto x) NOTHROW {
    using T = decltype(x);
    ASSERT_NE(x, std::numeric_limits<T>::min());
    T mask = x >> std::numeric_limits<T>::digits;
    return (x ^ mask) - mask;
}


constexpr auto abs(std::unsigned_integral auto x) noexcept { return x; }

template <arithmetic T, arithmetic M>
constexpr T mod(T a, M m) NOTHROW {
    ASSERT_GT(m, 0);
    T t = a % m;
    return t < 0 ? t + m : t;
}

template <arithmetic T, arithmetic M>
constexpr T add_mod(T a, M m) NOTHROW {
    ASSERT_GE(a, 0);
    ASSERT_GT(m, 0);
    return a < m ? a : a - m;
}

template <std::integral T>
T average(T a, T b) { return (a & b) + ((a ^ b) >> 1); }

template <typename T>
constexpr T lerpx(T a, T b, float t) { return a + t * (b - a); }

// factor is float, not uniform_t
// sometimes you feel like a lerp [0, 1] sometimes you lerp [0, 1)
template <typename T>
constexpr T lerp(T a, T b, float t) {
    ASSERT_GE_LE(t, 0, 1);
    return lerpx(a, b, t);
}


constexpr uint64_t exponent_mask_64 = 0x7ff0'0000'0000'0000;
constexpr uint64_t mantissa_mask_64 = 0x000f'ffff'ffff'ffff;
constexpr double max_normal_64 = std::bit_cast<double>(exponent_mask_64 - 1);
constexpr double min_normal_64 = std::bit_cast<double>(mantissa_mask_64 + 1);
constexpr double max_denorm_64 = std::bit_cast<double>(mantissa_mask_64);

constexpr uint32_t exponent_mask_32 = 0x7f80'0000;
constexpr uint32_t mantissa_mask_32 = 0x007f'ffff;
constexpr float max_normal_32 = std::bit_cast<float>(exponent_mask_32 - 1);
constexpr float min_normal_32 = std::bit_cast<float>(mantissa_mask_32 + 1);
constexpr float max_denorm_32 = std::bit_cast<float>(mantissa_mask_32);

// (-inf, inf) ↦ (-1.0, 1.0)
constexpr double frac(double x) NOTHROW {
    // todo - IF_ASSERTS_ON(WARN_IF(isdenorm(x)));
    ASSERT_GE(x, int64min);
    ASSERT_LE(x, int64max);
    return x - int64_t(x);
}

constexpr uint64_t d2b(std::same_as<double> auto x) noexcept {
    return std::bit_cast<uint64_t>(x);
}

constexpr double b2d(std::same_as<uint64_t> auto z) noexcept {
    return std::bit_cast<double>(z);
}

/// 2ᴺ and log₂X via type punning for exponents [-1023..1024]

//  1024 ⇆ inf                      7ff0'0000'0000'0000
//  1023 ↤ 1.7976931348623157e+308  7fef'ffff'ffff'ffff
//  1023 ⇆ 8.9884656743115795e+307  7fe0'0000'0000'0000
//     0 ⇆ 1.0                      3ff0'0000'0000'0000
// -1022 ⇆ 2.2250738585072014e-308  0010'0000'0000'0000  ⌊normal⌋
// -1023 ↤ 2.2250738585072009e-308  000f'ffff'ffff'ffff  ⌈denorm⌉
// -1023 ⇆ 0.0                      0000'0000'0000'0000

constexpr uint64_t encexp_(int64_t n) noexcept { return (n + 1023) << 52; }
constexpr int64_t decexp_(uint64_t z) noexcept { return (z >> 52) - 1023; }

// [-1023..1024] ↦ [0.0, inf]
constexpr double zpow2(int64_t n) NOTHROW {
    ASSERT_GE_LE(n, -1023, 1024);
    return b2d(encexp_(n));
}

// [0.0, inf] ↦ [-1023..1024]
constexpr int64_t zlog2(double x) NOTHROW {
    ASSERT_GE(x, 0);
    return decexp_(d2b(x));
}

/// approximate by interpolating

// [-1023, 1024] ↦ [0.0, inf]
// never underestimates on [-1022, 1024]
// collapses to ~0 on [-1023, -1022)
// https://www.desmos.com/calculator/3dvgxcnffg
constexpr double azpow2(double x) NOTHROW {
    ASSERT_GE_LE(x, -1023, 1024);
    return b2d(encexp_(x) + int64_t(frac(x) * double(mantissa_mask_64 + 1)));
}

// [0.0, inf] ↦ [-1023, 1024]
// never overestimates
// https://www.desmos.com/calculator/0gqawniv9g
constexpr double azlog2(double x) NOTHROW {
    ASSERT_GE(x, 0);
    uint64_t bits = d2b(x);
    constexpr double factor = 1.0 / (mantissa_mask_64 + 1);
    return decexp_(bits) + (bits & mantissa_mask_64) * factor;
}

// [-inf, inf] ↦ [0.0, 1.0]
// clamps aggressively to smooth corners
template <auto k=30>
constexpr float squash(float x) {
    auto softplus2 = [](float x) { return std::log2(1 + std::exp2(x)); };
    return 1 / float(k) * (softplus2(k * x) - softplus2(k * (x - 1)));
}

// kinky near high curvature
// almost monotonic, but a little degenerate (seems to sort itself out at high k)
// e.g. azsquash<3.8>(1.28) == 0.884868
//      azsquash<3.8>(1.35) == 0.876645
// https://www.desmos.com/calculator/93edknzvra
// ~1.5✕ speedup vs squash.  meh.
// fixme - extremes
template <auto k=30>
constexpr double azsquash(double x) {
    auto asoftplus2 = [](double x) {
        return azlog2(1 + azpow2(std::clamp(x, -1023.0, 1024.0)));
    };

    return 1 / double(k) * (asoftplus2(k * x) - asoftplus2(k * (x - 1)));
}

template <auto k=400>
constexpr float softstep(float x) {
    return 0.5f + std::atan(float(k) * x) / math::pi;
}

// [-1,1] ↦ [0,inf], 0 ↦ 1
// bilinear_pow(-x, k) = 1 / bilinear_pow(x, k)
inline float bilinear_pow(float x, float k) {
    return std::pow((1 + x) / (1 - x), k);
}

inline float bilinear_pow_inv(float y, float k) {
    float r = std::pow(y, 1 / k);
    return (r - 1) / (r + 1);
}

// note k=v and k=-v are not symmetric
// consider fp degeneracy
// maybe flip things around
template <auto k=0.5f>
constexpr float squircle(float x) NOTHROW {
    ASSERT_GE_LE(x, 0, 1);
    ASSERT_GE_LE(k, -1, 1);
    constexpr float p = std::sqrt((1 - k) / (1 + k));
    return 1 - std::pow(1 - std::pow(x, p), 1 / p);
}

constexpr float squircle(float x, float k=0.5) NOTHROW {
    ASSERT_GE_LE(x, 0, 1);
    ASSERT_GE_LE(k, -1, 1);
    float p = std::sqrt((1 - k) / (1 + k));
    return 1 - std::pow(1 - std::pow(x, p), 1 / p);
}

#ifdef __SIZEOF_INT128__

inline std::ostream &dump_uint128(std::ostream &os, __uint128_t n) {
    const char *prefix = "0x";
    for (int i = 3; i >= 0; i--) {
        os << prefix << hexos() << std::setw(8) << std::setfill('0')
           << uint32_t(n >> 32 * i);
        prefix = "'";
    }
    return os;
}

// i*numer/denom with overflow avoidance
constexpr uint64_t scale(uint64_t i, uint64_t numer, uint64_t denom) NOTHROW {
    ASSERT(denom != 0);

    __uint128_t i128 = i;
    __uint128_t result = i128 * numer / denom;

#if ASSERTS_ON
    if (result > 0xffff'ffff'ffff'ffffu) {
        std::ostringstream ss;
        dump_uint128(ss, result) << " doesn't fit in 64 bits";
        ERROR(ss.str());
    }
#endif

    return result;
}

#else
#pragma email "support@microsoft.com" "Plz implement __uint128_t"
#endif

/// amp_sum
/// operands amplify each other
/// ∑aᵢ + λ⋅∏aᵢ ⧸ ∑aᵢ | aᵢ >= 0, λ >= 0
/// do we want dim_sum?

constexpr double amp_sum(double a, double b,
                         multiplier amp = multiplier{1}) NOTHROW {
    ASSERT_GE(a, 0);
    ASSERT_GE(b, 0);
    ASSERT_GE(amp, 0);

    double sum = a + b;
    if (sum == 0) return 0;
    return sum + amp * a * b / sum;
}

// quadratic; more extreme, cheaper, sensitive to 0
constexpr double amp_sum(double a, double b, double c,
                         multiplier amp = multiplier{1}) NOTHROW {
    ASSERT_GE(a, 0);
    ASSERT_GE(b, 0);
    ASSERT_GE(c, 0);
    ASSERT_GE(amp, 0);

    double sum = a + b + c;
    if (sum == 0) return 0;
    return sum + amp * a * b * c / sum;
}

// linear; less extreme, more expensive, less sensitive to 0
constexpr double amp_sum_ex(double a, double b, double c,
                            multiplier amp = multiplier{1}) NOTHROW {
    ASSERT_GE(a, 0);
    ASSERT_GE(b, 0);
    ASSERT_GE(c, 0);
    ASSERT_GE(amp, 0);

    return (amp_sum(a, b, amp) +
            amp_sum(b, c, amp) +
            amp_sum(c, a, amp)) / 2;
}

template <std::unsigned_integral T>
constexpr T round_up(T value, T multiple) NOTHROW {
    ASSERT_GT(multiple, 0);
    return (value + (multiple - 1)) / multiple * multiple;
}

template <std::unsigned_integral T>
constexpr T round_down(T value, T multiple) NOTHROW {
    ASSERT_GT(multiple, 0);
    return value / multiple * multiple;
}

}

}
