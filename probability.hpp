// SPDX-License-Identifier: MIT
// Copyright (c) 2025 Joshua C Marshall

#pragma once

#include "success.hpp"
#include "entropy.hpp"
#include "reporting.hpp"

#include <algorithm>
#include <cmath>

namespace hjx {

class probability {
    float v;

public:
    explicit constexpr probability(float v) NOTHROW: v(v) { ASSERT_GE_LE(v, 0, 1); }

    // consider whether we really want float default behavior
    bool operator==(const probability&) const = default;
    auto operator<=>(const probability&) const = default;

    float get() const { return v; }
    void set(float v) { *this = probability(v); }

    probability operator&(const probability &o) const {
        return probability(v * o.v);
    }

    probability operator|(const probability &o) const {
        return probability(1.f - (1.f - v) * (1.f - o.v));
    }

    probability inv() const { return probability(1 - v); }

    probability bias(float bias) const {
        ASSERT_GE_LE(bias, -1, 1);

        // bias 0 returns p unchanged
        // bias 1.0 maps p to the top half of a circle r=1, c=(1,0)
        // bias -1.0 maps p to the bottom half of a circle r=1, c=(0,1)
        //
        // e.g. at full bias, 0.1 becomes 0.44 or 0.01
        //                    0.5 becomes 0.87 or 0.13
        //                    0.9 becomes 0.99 or 0.56
        //
        //      at half bias, 0.1 becomes 0.18 or 0.05
        //                    0.5 becomes 0.66 or 0.34
        //                    0.9 becomes 0.95 or 0.82
        //
        // at any bias, 0.0 stays 0.0 and 1.0 stays 1.0
        //
        // https://www.desmos.com/calculator/yueyruddan
        //
        // (if it's too expensive, try triangles with vertex on y = 1 - x)

        if (bias == 0) return probability(v);

        // fixme - semicirc still isn't perfect
        // e.g. probability(0.99).bias(-0.000001) > 0.99
        // this fix is a generic adhesive first-aid bandage
        if (bias > 0 && bias <  0.00001) bias =  0.00001;
        if (bias < 0 && bias > -0.00001) bias = -0.00001;

        auto semicirc = [](double v, double r) {
            // double for attempted stability
            return std::sqrt(r*r + (1-r)*(1-r) - (v-r)*(v-r));
        };

        float p;
        if (bias > 0) { double r = 1/bias;     p = 1 - r + semicirc(v, r); }
        else          { double r = 1/bias + 1; p = 1 - r - semicirc(v, r); }
        return probability(std::clamp(p, 0.f, 1.f));
    }

    probability scale(float factor) const {
        ASSERT_GE(factor, 0);
        float ret = 1 - powf(1 - v, factor);
        ASSERT_GE_LE(ret, 0, 1);
        return probability(ret);
    }

    friend bool operator<(uniform_t u, probability p) { return u < p.get(); }

    friend std::ostream &operator<<(std::ostream &os, probability p) {
        return os << p.v * 100 << '%';
    }
};

inline bool lucky(rand_t &rand, probability p) {
    return stud(rand) < p; // uniform_t < probability
}

inline bool lucky(rand_t &rand, float p) {
    return stud(rand) < probability(p);
}

// Luck Smoothing
// ==============
// True randomness is clumpy.  Sometimes you'll get brutal losing streaks,
// sometimes nigh-impossibly good ones.  Both can feel rigged when they're not.
//
// This slider sets how often you're willing to let those natural rare streaks
// happen.  If you're more averse to extreme luck, set the slider high and enjoy
// aggressive randomness smoothing.  (default 0.5? playtest)

// consider - sqrt(bias) might be smoother
//            maybe preprocess in UI (and default to 0.25?)

class elastic_rand {
    rand_t rand;
    float bias;
    //float rise_ps, fall_ps; todo - decay
    constexpr static float HIGH_BIAS = 1.5625;

public:
    const float smoothing;

    float effective_smoothing() const {
        // consider a gradual shift
        return std::fabs(bias) < HIGH_BIAS ? smoothing : 1.f;
    }

    elastic_rand(float smoothing, uint64_t seed)
        NOTHROW: rand(seed), bias(0), smoothing(smoothing)
    {
        ASSERT_GE_LE(smoothing, 0, 1);
    }

    probability bias_prob(probability p, elastic_rand &opp) const {
        float smooth = (effective_smoothing() + opp.effective_smoothing()) * 0.5f;
        return p.bias(std::clamp(bias - opp.bias, -1.f, 1.f) * smooth);
    }

#if 0
    bool lucky(probability p) {
        float smooth = effective_smoothing();
        probability newp = p.bias(std::clamp(bias, -1.f, 1.f) * smooth);
        bool success = stud(rand) < newp;
        bias += (p.get() - success) * smooth;
        return success;
    }

    // for contested rolls (affect and effect both parties' biases)
    bool lucky(probability p, elastic_rand &opp) {
        // geometric mean is a trap
        float smooth = (effective_smoothing() + opp.effective_smoothing()) * 0.5f;
        float combias = std::clamp(bias - opp.bias, -1.f, 1.f) * smooth;

        // Fine using this->rand, I think.  Just don't expose the bits to lua.
        bool success = stud(rand) < p.bias(combias);
        float bump = (p.get() - success) * smooth;

        bias += bump;
        opp.bias -= bump;

        return success;
    }
#endif

    success_t lucky(probability p) {
        float smooth = effective_smoothing();
        probability newp = p.bias(std::clamp(bias, -1.f, 1.f) * smooth);
        // consider API: newp.lucky(rand)
        float result = newp.get() - stud(rand); // p - [0,1) = (p-1,p]
        success_t s(result ? result : p.get() - 1); // (p-1,p] => [p-1,p]\0
        bias += (p.get() - s.is_success()) * smooth;
        return s;
    }

    // contested rolls
    success_t lucky(probability p, elastic_rand &opp) {
        // geometric mean is a trap
        float smooth = (effective_smoothing() + opp.effective_smoothing()) * 0.5f;
        float combias = std::clamp(bias - opp.bias, -1.f, 1.f) * smooth;

        // Fine using this->rand, I think.  Just don't expose the bits to lua.
        float result = p.bias(combias).get() - stud(rand);
        success_t s(result ? result : -1);
        float bump = (p.get() - s.is_success()) * smooth;

        bias += bump;
        opp.bias -= bump;

        return s;
    }

#if 0
    float roll() {
        probability p(stud(rand));
        return p.bias(std::clamp(bias, -1.f,  1.f) * effective_smoothing());
    }

    float roll(elastic_rand &opp) {
        // geometric mean is a trap
        float smooth = (effective_smoothing() + opp.effective_smoothing()) * 0.5f;
        float combias = std::clamp(bias - opp.bias, -1.f, 1.f) * smooth;
        return probability(stud(rand)).bias(combias);
    }

    constexpr static uint64_t skew_(uint64_t i, float bias) {
        ASSERT_GE_LE(bias, -1, 1);
        float p = probability(i * 0x1.0p-64).bias(bias);
        return std::min(p, 0.99999994f) * 0x1.0p+64;
    }

    uint64_t operator()() {
        // This has 99 stability issues, but it's rand so I think I don't care.
        return skew_(rand(), steal_bias());
    }
#endif

    float get_bias() const { return bias; }

    void set_bias(float b) {
        VERIFY(std::isfinite(b), "bad bias " << b, return);
        bias = b;
    }

    float steal_bias() {
        float booty = std::clamp(bias, -1.f, 1.f);
        bias -= booty;
        return booty;
    }

    probability steal_bias(probability p) {
        return p.bias(steal_bias()); // as if smoothing == 1
    }
};

}
