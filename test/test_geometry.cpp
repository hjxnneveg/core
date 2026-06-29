// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include "core/geometry.hpp"

#include "core/test/tests.hpp"

namespace hjx {

using namespace geom;

DEFINE_TEST(intersection) {
    // intersecting start
    TEST_EQ(intersect({0,0}, {1,1}, circle{{0,0}, 1}), 0);
    TEST_EQ(intersect({0,0}, {1,0}, circle{{0.5,0}, 1}), 0);
    TEST_EQ(intersect({0,0}, {1,0}, circle{{1,0}, 1}), 0);

    // head on
    TEST_EQFISH(intersect({0,0}, {4,0}, circle{{2,0}, 1}), 0.25);
    TEST_EQFISH(intersect({0,0}, {2,0}, circle{{2,0}, 1}), 0.5);

    // diagonal
    TEST_EQFISH(intersect({0,0}, {1,1}, circle{{1,1}, 1}), 1 - math::sqrt2/2);
    TEST_EQFISH(intersect({0,0}, {1,1}, circle{{1,1}, math::sqrt2/2}), 0.5);

    // tangent
    TEST_EQFISH(intersect({0,0}, {4,0}, circle{{2,1}, 1}), 0.5);

    // miss
    TEST_NAN(intersect({0,0}, {4,0}, circle{{2,2}, 1}));
    TEST_NAN(intersect({0,0}, {1,0}, circle{{5,0}, 1}));
    TEST_NAN(intersect({0,0}, {1,0}, circle{{-5,0}, 1}));

    // zeroish length
    TEST_EQ(intersect({3,3}, {3,3}, circle{{3,3}, 1}), 0);
    TEST_EQ(intersect({0,0}, {0,0}, circle{{0.5,0}, 1}), 0);
    TEST_EQ(intersect({0,0}, {0,1e-13}, circle{{0.5,0}, 1}), 0);
    TEST_EQ(intersect({0,1e-13}, {0,0}, circle{{0.5,0}, 1}), 0);
    TEST_NAN(intersect({0,0}, {0,0}, circle{{5,0}, 1}));
    TEST_NAN(intersect({0,0}, {0,1e-13}, circle{{5,0}, 1}));
    TEST_NAN(intersect({0,1e-13}, {0,0}, circle{{5,0}, 1}));

    // intersecting finish
    TEST_EQFISH(intersect({0,0}, {1,0}, circle{{2,0}, 1}), 1);
}

#if 0
DEFINE_TEST(geometry) {
    using namespace geom;
    // todo
    obb b(xy(0, 0), 0.0_deg, 0.51, 0.49);
    obb s(xy(0, -1), 0.0_deg, 0.51, 0.49);
    obb t(xy(0, 0), 45.0_deg, 0.51, 0.49);
    circle c(xy(0, -1), 0.5);
    TEST(!collision(b, s));
    TEST(collision(t, b));
    TEST(!collision(b, c));
    TEST(collision(s, t));
    TEST(collision(s, c));
    TEST(collision(c, t));
}
#endif

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:geometry", argv);
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
