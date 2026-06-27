// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "reporting.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <concepts>

namespace hjx {

namespace entropy {

// Newton-Raphson
constexpr uint64_t mul_inv(uint64_t z) {
    ASSERT(z & 1);
    uint64_t inv = 1;
    for (int i = 0; i < 6; i++) inv *= 2 - z * inv;
    ASSERT_EQ(z * inv, 1);
    return inv;
}

constexpr uint64_t unxorshift(uint64_t z, int shift) {
    uint64_t ret = z;
    for (int i = shift; i < 64; i += shift) ret ^= z >> i;
    ASSERT_EQ(ret ^ (ret >> shift), z);
    return ret;
}

constexpr uint64_t goldseed() { return 0x9e37'79b9'7f4a'7c15; }

struct splitmix {
    constexpr static uint64_t shift1 = 30;
    constexpr static uint64_t mult2 = 0xbf58'476d'1ce4'e5b9;
    constexpr static uint64_t shift3 = 27;
    constexpr static uint64_t mult4 = 0x94d0'49bb'1331'11eb;
    constexpr static uint64_t shift5 = 31;

    constexpr static uint64_t hash(uint64_t z) {
        z ^= z >> shift1;
        z *= mult2;
        z ^= z >> shift3;
        z *= mult4;
        z ^= z >> shift5;
        return z;
    }

    static uint64_t unhash(uint64_t z) {
        z = unxorshift(z, shift5);
        z *= mul_inv(mult4);
        z = unxorshift(z, shift3);
        z *= mul_inv(mult2);
        z = unxorshift(z, shift1);
        return z;
    }
};

template <auto step>
class direct_rng {
    uint64_t state_;

public:
    explicit direct_rng(uint64_t seed=1) noexcept: state_(seed ? seed : 1) {}

    void seed(uint64_t seed) { state_ = seed ? seed : 1; }
    uint64_t state() const { return state_; }
    void next() { state_ = step(state_); }
    uint64_t get() const { return state_; }

    uint64_t operator()() { next(); return get(); }

    direct_rng(direct_rng&&) = delete;
    direct_rng(const direct_rng&) = delete;
    direct_rng &operator=(direct_rng&&) = delete;
    direct_rng &operator=(const direct_rng&) = delete;
};

constexpr uint64_t xorshift64(uint64_t u) {
    u ^= u << 13;
    u ^= u >> 7;
    u ^= u << 17;
    return u;
}

constexpr uint64_t xorshift64star(uint64_t u) {
    u ^= u >> 12;
    u ^= u << 25;
    u ^= u >> 27;
    return u * 0x2545'f491'4f6c'dd1du;
}

constexpr uint64_t xorshift_79(uint64_t u) {
    u ^= u << 7;
    u ^= u >> 9;
    return u;
}

constexpr uint64_t murmur3mix(uint64_t u) {
    u ^= u >> 33;
    u *= 0xff51'afd7'ed55'8ccd;
    u ^= u >> 33;
    u *= 0xc4ce'b9fe'1a85'ec53;
    u ^= u >> 33;
    return u;
}

// "Better Bit Mixing - Improving on MurmurHash3's 64-bit Finalizer"
// author David Stafford
// https://zimbry.blogspot.com/2011/09/better-bit-mixing-improving-on.html

constexpr uint64_t stafford01(uint64_t u) {
    u ^= u >> 31;
    u *= 0x7fb5'd329'728e'a185;
    u ^= u >> 27;
    u *= 0x81da'def4'bc2d'd44d;
    u ^= u >> 33;
    return u;
}

constexpr uint64_t stafford02(uint64_t u) {
    u ^= u >> 33;
    u *= 0x64dd'8148'2cbd'31d7;
    u ^= u >> 31;
    u *= 0xe36a'a5c6'1361'2997;
    u ^= u >> 31;
    return u;
}

constexpr uint64_t stafford03(uint64_t u) {
    u ^= u >> 31;
    u *= 0x99bc'f682'2b23'ca35;
    u ^= u >> 30;
    u *= 0x1402'0a57'acce'd8b7;
    u ^= u >> 33;
    return u;
}

constexpr uint64_t fasthash(uint64_t u) {
    u ^= u >> 23;
    u *= 0x2127'599b'f432'5c37;
    u ^= u >> 47;
    return u;
}

constexpr uint32_t lowbias32(uint32_t u) {
    u ^= u >> 16;
    u *= 0x7feb352d;
    u ^= u >> 15;
    u *= 0x846ca68b;
    u ^= u >> 16;
    return u;
}

constexpr uint32_t fmix32(uint32_t u) {
    u ^= u >> 16;
    u *= 0x85ebca6b;
    u ^= u >> 13;
    u *= 0xc2b2ae35;
    u ^= u >> 16;
    return u;
}

constexpr uint32_t rol32(uint32_t u, uint32_t n) {
    ASSERT_GE_LE(n, 0, 32);
    return u << n | u >> (32 - n);
}

constexpr void chacha_qr(uint32_t u[4]) {
    u[0] += u[1]; u[3] ^= u[0]; u[3] = rol32(u[3], 16);
    u[2] += u[3]; u[1] ^= u[2]; u[1] = rol32(u[1], 12);
    u[0] += u[1]; u[3] ^= u[0]; u[3] = rol32(u[3], 8);
    u[2] += u[3]; u[1] ^= u[2]; u[1] = rol32(u[1], 7);
}

constexpr uint32_t xorshift128(uint32_t u[4]) {
    uint32_t t = u[3];
    uint32_t s = u[0];
    u[3] = u[2];
    u[2] = u[1];
    u[1] = s;
    t ^= t << 11;
    t ^= t >> 8;
    return u[0] = t ^ s ^ (s >> 19);
}

}

using rand_t = entropy::direct_rng<entropy::xorshift64>;


// [0.f, 1.f)
// hard going in, easy coming out
// uniform about 0.5f - ε/2
class uniform_t {
    // consider storing as [1.f, 2.f) to enforce quantization and avoid denorms
    float storage;

public:
    constexpr explicit uniform_t(float u)
        NOTHROW: storage(u) { ASSERT_GE_LT(u, 0, 1); }

    operator float() const { return storage; }
};


/// monotonic "standard" uniform distributions
///
/// [0.f, 1.f] only has 2^23+1 (~8 million) values, so endpoints aren't rare
///
/// note LCGs won't give 0,
/// but (0..2^64-1] yields same range as [0..2^64-1]

constexpr uint64_t STUD_ULP = 0x0000'0200'0000'0000;
constexpr uint64_t STUD_LOWMAX = 0xffff'ffff'ffff'ffff - STUD_ULP + 1;

// [0..2^64-1] ↦ [1.0, 2.0) or (1.0, 2.0]
constexpr float uniform12_(uint64_t u, bool nudge) {
    return std::bit_cast<float>(int32_t((127 << 23 | u >> 41) + nudge));
}

// [0..2^64-1] ↦ [1.0, 2.0) or (1.0, 2.0]
constexpr double uniform12d_(uint64_t u, bool nudge) {
    return std::bit_cast<double>((int64_t(1023) << 52 | u >> 12) + nudge);
}

// [0..2^64-1] ↦ [0.0, 1.0)
constexpr uniform_t stud(uint64_t u) {
    return uniform_t(uniform12_(u, false) - 1);
}

// [0..2^64-1] ↦ [0.0, 1.0)
constexpr double doublestud(uint64_t u) {
    return uniform12d_(u, false) - 1;
}

// [0..2^64-1] ↦ (0.0, 1.0]
constexpr float stud_oc(uint64_t u) {
    return uniform12_(u, true) - 1;
}

// closed 1D unit ball with a gap at the origin
// [0..2^64-1] ↦ [-1.0, 1.0] \ 0.0
constexpr float club_gap(uint64_t u) {
    // fragile - assumes stud_oc uses the leftmost 23 bits
    uint32_t bits = (u >> (32 - 23) & 0x8000'0000) | std::bit_cast<uint32_t>(stud_oc(u));
    return std::bit_cast<float>(bits);
}

// exponential distribution sampling: anti-exploitation lifetime generation
constexpr float memoryless(float u) {
    // bound  0.0        ↦ 0.0
    // "min" ~0.00000003 ↦ 0.0000000596
    // mean   0.5        ↦ 0.693
    // "max"  0.99999995 ↦ 16.6
    // bound  1.0        ↦ inf

    ASSERT_GE_LE(u, 0, 1);
    return -logf(1 - u);
}

inline uniform_t stud(auto &rand) { return stud(rand()); }

// normal enough (range ±~3.4641)
constexpr float normal_approx(auto &rand) {
    float sum = stud(rand) + stud(rand)
        + stud_oc(rand) + stud_oc(rand); // indulgence
    return (sum - 2) * sqrtf(3);
}


uint64_t hash(const void*, size_t) noexcept;

constexpr uint64_t hash(uint64_t z) { return entropy::murmur3mix(z); }

constexpr uint64_t hash(std::integral auto a, std::integral auto b) {
    if constexpr (sizeof(a) > 4 || sizeof(b) > 4)
        return (hash(uint64_t(a)) + entropy::goldseed()) ^ hash(uint64_t(b));
    else
        return hash(uint64_t(a) << 32 ^ uint64_t(b));
}

constexpr uint64_t hash(const std::string_view v) { return hash(v.data(), v.size()); }

// [0, ub)
constexpr uint32_t scale_seed_32(std::same_as<std::uint64_t> auto seed, uint32_t ub) {
    return (seed >> 32) * ub >> 32;
}

template <std::integral T>
inline T randint(auto &&rand, T min, T max) { // [min, max]
    uint64_t range = uint64_t(max) - uint64_t(min);
    ASSERT_LT(range, 0xffff'ffff);
    return uint64_t(min) + scale_seed_32(rand(), range + 1);
}

template <std::integral T>
inline T randidx(auto &&rand, T sz) { // [0, sz-1]
    ASSERT(sz);
    return randint(std::forward<decltype(rand)>(rand), 0, sz - 1);
}

template <std::ranges::random_access_range R>
requires std::permutable<std::ranges::iterator_t<R>>
void shuffle(R &&range, auto &&rand) {
    // Fisher-Yates
    auto first = std::ranges::begin(range);
    auto N = std::ranges::distance(range);
    auto &r = rand;
    for (auto i = N - 1; i; i--)
        std::ranges::iter_swap(first + i, first + randint(r, decltype(i){0}, i));
}

inline uint64_t new_seed() {
    auto now = std::chrono::steady_clock::now();
    return hash(now.time_since_epoch().count());
}

}
