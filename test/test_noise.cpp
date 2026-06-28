// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/noise.hpp"

#include "core/test/tests.hpp"

namespace hjx {

DEFINE_TEST(noise_conditioner) {
    using Cond = noise::conditioner<0.f, 1.f>;
    Cond c{[]{ static int bound = 6; return bound++; }, 2};
    TEST_EQ(c(4.5), 1.0);
    TEST_EQ(c(5.0), 1.0);
    TEST_EQ(c(5.5), 0.5);
    TEST_EQ(c(6.0), 0.0);
    TEST_EQ(c(6.5), 0.5);
    TEST_EQ(c(7.0), 1.0);
    TEST_EQ(c(7.5), 0.5);
    TEST_EQ(c(8.0), 0.0);
    TEST_EQ(c(8.5), 0.0);

#if 0
    using PCond = noise::conditioner<-1.f, 1.f>;

    rand_t rand(test::harness::seed());
    static siv::BasicPerlinNoise<float> perl(test::harness::seed());
    // zeroes at ints
    float y = 0.1 + 0.8 * stud(rand);
    float z = 0.1 + 0.8 * stud(rand);
    auto gen = [&](float x) { return perl.noise3D(9999 * x, y, z); };

    PCond pc{[&] { return gen(stud(rand)); }};

    float min = inf;
    float max = -inf;
    for (int i = 0; i < 40000; i++) {
        float noise = gen(i * 1.618 + stud(rand));
        float nice = pc(noise);
        if (nice < min) min = nice;
        if (nice > max) max = nice;
    }
    DUMPI(min << ", " << max);
#endif
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:noise", argv);
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
