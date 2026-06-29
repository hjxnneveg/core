// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include "core/geometry.hpp"

#include "core/test/tests.hpp"

namespace hjx {

using namespace geom;

DEFINE_TEST(intersect_ray) {
    // intersecting start
    TEST_EQ(intersect({0,0}, {1,1}, circle{{0,0}, 1}), 0);
    TEST_EQ(intersect({0,0}, {1,0}, circle{{0.5,0}, 1}), 0);
    TEST_EQ(intersect({0,0.7}, {-1,-1}, circle{{0.5,0}, 1}), 0);
    TEST_EQ(intersect({0,0}, {1,0}, circle{{1,0}, 1}), 0);

    // head on
    TEST_EQFISH(intersect({0,0}, {4,0}, circle{{2,0}, 1}), 0.25);
    TEST_EQFISH(intersect({0,0}, {2,0}, circle{{2,0}, 1}), 0.5);
    TEST_EQFISH(intersect({4,0}, {-4,0}, circle{{2,0}, 1}), 0.25);

    // diagonal
    TEST_EQFISH(intersect({0,0}, {1,1}, circle{{1,1}, 1}), 1 - math::sqrt2/2);
    TEST_EQFISH(intersect({0,0}, {1,1}, circle{{1,1}, math::sqrt2/2}), 0.5);

    // tangent
    TEST_EQFISH(intersect({0,0}, {4,0}, circle{{2,1}, 1}), 0.5);

    // miss
    TEST_NAN(intersect({0,0}, {4,0}, circle{{2,2}, 1}));
    TEST_NAN(intersect({0,0}, {1,0}, circle{{5,0}, 1}));
    TEST_NAN(intersect({0,0}, {1,0}, circle{{-5,0}, 1}));

    // near miss
    TEST_NAN(intersect({0,0}, {4,0}, circle{{2,1.01}, 1}));
    TEST_NAN(intersect({0,0}, {1,0}, circle{{2,0}, 0.9}));

    // zeroish length
    TEST_EQ(intersect({3,3}, {0,0}, circle{{3,3}, 1}), 0);
    TEST_EQ(intersect({0,0}, {0,0}, circle{{0.5,0}, 1}), 0);
    TEST_EQ(intersect({0,0}, {0,zeroish/2}, circle{{0.5,0}, 1}), 0);
    TEST_EQ(intersect({0,zeroish/2}, {0,-zeroish/2}, circle{{0.5,0}, 1}), 0);
    TEST_NAN(intersect({0,0}, {0,0}, circle{{5,0}, 1}));
    TEST_NAN(intersect({0,0}, {0,zeroish/2}, circle{{5,0}, 1}));
    TEST_NAN(intersect({0,zeroish/2}, {0,-zeroish/2}, circle{{5,0}, 1}));

    // intersecting finish
    TEST_EQFISH(intersect({0,0}, {1,0}, circle{{2,0}, 1}), 1);
}

#define TEST_EQXYISH(a, b)                      \
    do {                                        \
        xy ac = a;                              \
        xy bc = b;                              \
        TEST_EQFISH(ac.x(), bc.x());            \
        TEST_EQFISH(ac.y(), bc.y());            \
    } while (false)

DEFINE_TEST(intersect_circle) {
    // head on, equal radii
    intersection r = intersect(circle{{0,0},1}, {4,0}, circle{{4,0},1});
    TEST_EQXYISH(r.impact, xy(3,0));
    TEST_EQXYISH(r.normal, xy(-1,0));
    TEST_EQFISH(r.t, 0.5);

    // 3-4-5
    r = intersect(circle{{0,0},1}, {10,0}, circle{{8,4},4});
    TEST_EQXYISH(r.impact, xy(5.6,0.8));
    TEST_EQXYISH(r.normal, xy(-0.6,-0.8));
    TEST_EQFISH(r.t, 0.5);

    // overlapping start
    r = intersect(circle{{0,0},1}, {1,0}, circle{{1,0},1});
    TEST_REPR(r, "[(0,0) t=0 ⟂(-1,0)]"); // exact

    // contact finish
    r = intersect(circle{{0,0},1}, {2,0}, circle{{4,0},1});
    TEST_EQXYISH(r.impact, xy(3,0));
    TEST_EQXYISH(r.normal, xy(-1,0));
    TEST_EQFISH(r.t, 1);

    // coincident centers: off==0 -> fallback normal = -deltâ, unit not NaN
    r = intersect(circle{{0,0},1}, {2,0}, circle{{0,0},1});
    TEST_REPR(r, "[(-1,0) t=0 ⟂(-1,0)]");

    // miss
    r = intersect(circle{{0,0},1}, {4,0}, circle{{2,5},1});
    TEST_NOT(r.impact);
    TEST_NOT(r.normal);
    TEST_NAN(r.t);
    TEST_REPR(r, "nil");
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
