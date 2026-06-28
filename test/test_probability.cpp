// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/probability.hpp"

#include "core/test/tests.hpp"

namespace hjx {

DEFINE_TEST(lucky) {
    uint64_t seed = test::harness::seed();
    rand_t rand(seed);

    for (size_t i = 0; i < 999'999; i++) {
        TEST_MSG(lucky(rand, probability(1.0)), "not lucky with " << seed);
        TEST_MSG(!lucky(rand, probability(0.0)), "too lucky with " << seed);
    }

    elastic_rand cheater(1.0, seed);

    for (int reps = 0; reps < 999; reps++) {
        const int N = 1000;
        probability p(stud(rand));
        float expected = p.get() * N;
        float actual = 0;

        for (int i = 0; i < N; i++) if (cheater.lucky(p).is_success()) actual++;

        // I haven't seen this fail at maximum smoothing (1.0), but no promises.
        // As long as drift stays under 10, we're within 1 percentage point over
        // 1000 trials.
        const int max_drift_per_1000 = 7;
        TEST_MSG(std::abs(actual - expected) < max_drift_per_1000,
                 p << " : " << actual << " vs " << expected);
    }

#if 0
    TEST_EQ(elastic_rand::skew_(0, -1), 0);
    TEST_EQ(elastic_rand::skew_(0, 0), 0);
    TEST_EQ(elastic_rand::skew_(0, 1), 0);
    // we get 24 bits before trouble
    TEST_EQ(elastic_rand::skew_(0xffff'ff00'0000'0000, 0),
            0xffff'ff00'0000'0000);
    // "bad" behavior, but want to know if it changes
    TEST_EQ(elastic_rand::skew_(0xffff'ffff'ffff'ffff, 0),
            0xffff'ff00'0000'0000);
#endif

    // todo - test long-term bias
}

DEFINE_TEST(probability) {
    probability prob(0);
    prob.set(0.4);
    TEST_EQ(prob, probability(0.4));

    TEST_REPR(probability(0.0),   "0%");
    TEST_REPR(probability(0.5),  "50%");
    TEST_REPR(probability(1.0), "100%");

    TEST_EQ(probability(0.5) & probability(0.5), probability(0.25));
    TEST_EQ(probability(0.5) | probability(0.5), probability(0.75));

    auto pb = [](float p, float bias) {
        return std::round(probability(p).bias(bias).get() * 100);
    };

    TEST_EQ(pb(0.1, -1.0),  1);
    TEST_EQ(pb(0.1, -0.5),  5);
    TEST_EQ(pb(0.1,  0.0), 10);
    TEST_EQ(pb(0.1,  0.5), 18);
    TEST_EQ(pb(0.1,  1.0), 44);

    TEST_EQ(pb(0.5, -1.0), 13);
    TEST_EQ(pb(0.5, -0.5), 34);
    TEST_EQ(pb(0.5,  0.0), 50);
    TEST_EQ(pb(0.5,  0.5), 66);
    TEST_EQ(pb(0.5,  1.0), 87);

    TEST_EQ(pb(0.9, -1.0), 56);
    TEST_EQ(pb(0.9, -0.5), 82);
    TEST_EQ(pb(0.9,  0.0), 90);
    TEST_EQ(pb(0.9,  0.5), 95);
    TEST_EQ(pb(0.9,  1.0), 99);

    rand_t rand(test::harness::seed());
    for (int i = 0; i < 99999; i++) {
        probability p(stud(rand));
        float b1 = stud(rand);
        float b2 = 1 - b1;
        probability p1 = p.bias(b1);
        probability p2 = p.bias(b2);
        TEST_MSG(p1 >= p, "trouble " << p << " (bias " << b1 << ") => " << p1);
        TEST_MSG(p2 >= p, "trouble " << p << " (bias " << b2 << ") => " << p2);
    }

    TEST_EQ(probability(0).bias(1), probability(0));
    TEST_EQ(probability(0).bias(-1), probability(0));
    TEST_EQ(probability(1).bias(1), probability(1));
    TEST_EQ(probability(1).bias(-1), probability(1));

    for (int i = 0; i < 999; i++) {
        float b = stud(rand) * 2 - 1;
        TEST_EQ(probability(0).bias(b), probability(0));
        TEST_EQ(probability(1).bias(b), probability(1));
    }

    TEST_LE(probability(0.99).bias(-0.000001), probability(0.99));
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:probability", argv);
        return 0;
    }
    catch (const char *s) {
        if (strcmp(s, FAIL_MSG))
            std::cout << "unexpected exception " << s << std::endl;
    }
    catch (...) {
        std::cout << "unknown exception" << std::endl;
    }

    return 1;
}
