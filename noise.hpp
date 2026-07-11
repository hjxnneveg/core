// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#pragma once

#include "core/logging.hpp"
#include "uptime.hpp"
#include "reporting.hpp"
#include "entropy.hpp"
#include "coords.hpp"

#include "external/PerlinNoise.hpp"
#include "external/stegu/simplexnoise1234.hpp"

#include <cmath>
#include <concepts>

namespace hjx {

inline float stud(duration d) {
    static siv::BasicPerlinNoise<float> perl(time::now_ns());
    float v = perl.noise1D(time::uptime().seconds() / to_seconds(d));
    static float min = 1, max = -1;
    if (v < min) min = v;
    if (v > max) max = v;
    float scale = 2 / (max - min);
    v = v * scale - min - 1;
    float ret = v * 0.5f + 0.5f;
    ASSERT_GE_LE(ret, 0, 1);
    return ret;
}

}

namespace hjx::noise {

class linear {
    uint64_t seed_;

    float calc_vertex(xy c) const {
        ASSERT_EQ(c.x(), std::trunc(c.x()));
        ASSERT_EQ(c.y(), std::trunc(c.y()));
        return 2 * stud(hash(c.id() + seed_)) - 1;
    }

    float weight(xy v, xy p) const {
        float d = cart_dist(v, p);
        if (d < 1) return (1 - d) * (1 - d);
        return 0;
    }

public:
    linear(uint64_t seed): seed_(seed) {}

    float get(xy c) {
        float x1 = std::trunc(c.x());
        float y1 = std::trunc(c.y());
        float x2 = c.x() > 0 ? x1 + 1 : x1 - 1;
        float y2 = c.y() > 0 ? y1 + 1 : y1 - 1;

        xy c11(x1, y1); xy c12(x1, y2); xy c21(x2, y1); xy c22(x2, y2);

        float w11 = weight(c, c11);
        float w12 = weight(c, c12);
        float w21 = weight(c, c21);
        float w22 = weight(c, c22);

        return (w11 * calc_vertex(c11) +
                w12 * calc_vertex(c12) +
                w21 * calc_vertex(c21) +
                w22 * calc_vertex(c22)) / (w11 + w12 + w21 + w22);
    }
};


class perlin {
    siv::BasicPerlinNoise<float> perl;
    static constexpr float default_offset = 0.25;

    static uint32_t mutate(uint64_t seed) { return seed & seed >> 32; }

public:
    perlin(uint64_t seed): perl(mutate(seed)) {}

    void reseed(uint64_t seed) { perl.reseed(seed); }

    float operator()(float x) const {
        return perl.noise3D(x, default_offset, default_offset);
    }

    float operator()(float x, float y) const {
        return perl.noise3D(x, y, default_offset);
    }

    float operator()(xy p) const { return (*this)(p.x(), p.y()); }

    float operator()(float x, float y, float z) const {
        return perl.noise3D(x, y, z);
    }

    float octaves(float x,
                  std::integral auto n,
                  std::floating_point auto persistence) const {
        return perl.octave3D(x, default_offset, default_offset, n, persistence);
    }

    float octaves(float x, float y,
                  std::integral auto n,
                  std::floating_point auto persistence) const {
        return perl.octave3D(x, y, default_offset, n, persistence);
    }

    float octaves(xy p,
                  std::integral auto n,
                  std::floating_point auto persistence) const {
        return octaves(p.x(), p.y(), n, persistence);
    }

    float octaves(float x, float y, float z,
                  std::integral auto n,
                  std::floating_point auto persistence) const {
        return perl.octave3D(x, y, z, n, persistence);
    }
};


class simplex {
    unsigned char perm[512];

    void permute(auto &&rand) {
        std::iota(perm, perm + 256, 0);
        shuffle(std::span(perm, perm + 256), std::forward<decltype(rand)>(rand));
        std::memcpy(perm + 256, perm, 256);
    }

public:
    simplex(uint64_t seed) { permute(rand_t(seed)); }

    float operator()(float a) const {
        return stegu::snoise1(perm, a);
    }

    float operator()(float a, float b) const {
        return stegu::snoise2(perm, a, b);
    }

    float operator()(float a, float b, float c) const {
        return stegu::snoise3(perm, a, b, c);
    }

    float operator()(float a, float b, float c, float d) const {
        return stegu::snoise4(perm, a, b, c, d);
    }

    float operator()(xy p) const {
        return stegu::snoise2(perm, p.x(), p.y());
    }

    // too smooth near origin, like perlin
    float octaves(float a, float b,
                  std::integral auto n, std::floating_point auto persistence) const {
        ASSERT_GT(n, 0);

        float ret = 0;
        float amplitude = 1;

        while (n--) {
            ret += (*this)(a, b) * amplitude;
            a *= 2;
            b *= 2;
            amplitude *= persistence;
        }

        return ret;
    }

    float octaves(xy p, std::integral auto n, std::floating_point auto persistence) const {
        ASSERT_GT(n, 0);

        float ret = 0;
        float amplitude = 1;

        while (n--) {
            ret += (*this)(p) * amplitude;
            p *= 2;
            amplitude *= persistence;
        }

        return ret;
    }
};


// consider - hill-climber to determine per-seed min/max
template <float Min, float Max>
class conditioner {
public:
    static constexpr float min = Min;
    static constexpr float max = Max;

private:
    float low = inf;
    float high = -inf;

public:
    conditioner(auto &&generator, int iterations=10000) {
        while (iterations--) {
            float v = generator();
            if (v < low) low = v;
            if (v > high) high = v;
        }

        ASSERT_LT(low, high);
    }

    float operator()(float v) {
        // fold extremes
        if (v < low) v = low + low - v;
        else if (v > high) v = high + high - v;

        float norm = std::clamp((v - low) / (high - low), 0.f, 1.f);
        return min + norm * (max - min);
    }
};

}
