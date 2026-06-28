// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/coords.hpp"

#include "core/test/tests.hpp"

namespace hjx {

DEFINE_TEST(coords) {
    // todo
    TEST_NOT(xy());
    TEST_EQ(xy(), xy());
    TEST_NOT(xy() * 0);
    TEST_EQ(xy() * 0, xy() * 6);

    TEST_EQ(xy(0, 0), xy(float(-0.f), float(-0.f)));

    TEST_NE(xy(0, 0), xy());
    TEST_EQ(xy(1, 2), xy(1, 2));
    TEST_NE(xy(), xy(1, 2));
    TEST_EQ(xy(0, 1), xy(0, 1));

    TEST_EQ(xy(3, 4).abs(), 5);
    TEST_EQFISH(polar(xy(1, 1)).r, sqrt(2));

    qrs p;
    TEST_NOT(p);
    p = qrs(1.0, 2.0);
    TEST(p);
    qrs q(p);
    TEST_EQ(q, p);

    TEST_REPR(qrs(), "[dud]");
    TEST_REPR(qrs(0, 0), "[0,0,0]");

    TEST_EQ(qrs(direction::none),  qrs( 0,  0));
    TEST_EQ(qrs(direction::north), qrs( 0, -1));
    TEST_EQ(qrs(direction::ne),    qrs( 1, -1));
    TEST_EQ(qrs(direction::se),    qrs( 1,  0));
    TEST_EQ(qrs(direction::south), qrs( 0,  1));
    TEST_EQ(qrs(direction::sw),    qrs(-1,  1));
    TEST_EQ(qrs(direction::nw),    qrs(-1,  0));

    TEST_THROW(qrs(direction(99)));
}

DEFINE_TEST(qrs_dist) {
    TEST_EQ(qrs(1,  0).abs(), 1);
    TEST_EQ(qrs(2, -2).abs(), 2);
    TEST_EQ(qrs(3, -3).abs(), 3);
    TEST_EQ(qrs(4,  0).abs(), 4);

    auto pretty_close = [](float a, float b) {
        return std::abs(a - b) < 0.0001;
    };

    rand_t rand(test::harness::seed());
    for (int i = 0; i < 9999; i++) {
        qrs a(randint(rand, -99, 99), randint(rand, -99, 99));
        qrs b(randint(rand, -99, 99), randint(rand, -99, 99));
        float d1 = cart_dist(a, b);
        float d2 = sqrt(square_dist(a, b));
        TEST_MSG(pretty_close(d1, d2), a << ' ' << b << ' ' << d1 - d2);
    }
}

DEFINE_TEST(qrs_rotate) {
    TEST_EQ(qrs( 0, -3).rotate_l(), qrs(-3,  0));
    TEST_EQ(qrs( 3, -3).rotate_l(), qrs( 0, -3));
    TEST_EQ(qrs( 3,  0).rotate_l(), qrs( 3, -3));
    TEST_EQ(qrs( 0,  3).rotate_l(), qrs( 3,  0));
    TEST_EQ(qrs(-3,  3).rotate_l(), qrs( 0,  3));
    TEST_EQ(qrs(-3,  0).rotate_l(), qrs(-3,  3));

    TEST_EQ(qrs( 0, -3).rotate_r(), qrs( 3, -3));
    TEST_EQ(qrs( 3, -3).rotate_r(), qrs( 3,  0));
    TEST_EQ(qrs( 3,  0).rotate_r(), qrs( 0,  3));
    TEST_EQ(qrs( 0,  3).rotate_r(), qrs(-3,  3));
    TEST_EQ(qrs(-3,  3).rotate_r(), qrs(-3,  0));
    TEST_EQ(qrs(-3,  0).rotate_r(), qrs( 0, -3));
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:coords", argv);
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
