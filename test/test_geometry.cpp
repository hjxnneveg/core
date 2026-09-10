// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include <core/geometry.hpp>

#include <core/test/tests.hpp>

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
    TEST_REPR(r, "[(1,0) t=0 ⟂(-1,0)]"); // exact

    r = intersect(circle{{0,0},1}, {2,0}, circle{{-0.1,0},1});
    TEST_REPR(r, "[(-1,0) t=0 ⟂(1,0)]");

    r = intersect(circle{{0,0},1}, {2,0}, circle{{0,0.1},1});
    TEST_REPR(r, "[(0,1) t=0 ⟂(0,-1)]");

    // contact finish
    r = intersect(circle{{0,0},1}, {2,0}, circle{{4,0},1});
    TEST_EQXYISH(r.impact, xy(3,0));
    TEST_EQXYISH(r.normal, xy(-1,0));
    TEST_EQFISH(r.t, 1);

    // miss
    r = intersect(circle{{0,0},1}, {4,0}, circle{{2,5},1});
    TEST_NOT(r.impact);
    TEST_NOT(r.normal);
    TEST_NAN(r.t);
    TEST_REPR(r, "nil");
}

DEFINE_TEST(intersect_ray_obb) {
    obb ax{{4,0}, {1,1}, 0_deg};
    obb rot{{0,0}, {1,1}, 0.6, 0.8}; // 3-4-5 rotation

    // head on
    TEST_EQFISH(intersect({0,0}, {4,0}, ax), 0.75);
    TEST_EQFISH(intersect({0,0}, {6,0}, ax), 0.5);

    // rotated head-on along the box's own axis: path length 10
    TEST_EQFISH(intersect({-3,-4}, {6,8}, rot), 0.4);

    // started inside
    TEST_EQ(intersect({4,0.5}, {1,0}, ax), 0);

    // contact at finish
    TEST_EQFISH(intersect({0,0}, {3,0}, ax), 1);

    // miss: parallel outside / stops short / slab reject
    TEST_NAN(intersect({0,2}, {8,0}, ax));
    TEST_NAN(intersect({0,0}, {2,0}, ax));
    TEST_NAN(intersect({0,0}, {4,4}, obb{{5,1}, {1,1}, 1, 0}));

    // just nicks
    TEST_EQFISH(intersect({0,0}, {4,4}, obb{{4,2}, {1,1}, 1, 0}), 0.75);

    // zero length
    TEST_EQ(intersect({4,0.5}, {0,0}, ax), 0);
    TEST_NAN(intersect({0,0}, {0,0}, ax));
}

DEFINE_TEST(intersect_circle_obb) {
    obb ax{{4,0}, {1,1}, 0_deg};
    obb org{{0,0}, {1,1}, 90_deg};
    obb rot{{0,0}, {1,1}, 0.6, 0.8}; // 3-4-5 rotation

    // face head-on: inflated face x=2, contact on the real face
    intersection r = intersect(circle{{0, 0}, 1}, {4, 0}, ax);
    TEST(r);
    TEST_EQXYISH(r.impact, xy(3,0));
    TEST_EQXYISH(r.normal, xy(-1,0));
    TEST_EQFISH(r.t, 0.5);

    // corner arc: enters x-slab past the top face, rolls onto the (3,1) corner
    r = intersect(circle{{0,1.6},1}, {8,0}, ax);
    TEST_EQXYISH(r.impact, xy(3, 1));
    TEST_EQXYISH(r.normal, xy(-0.8, 0.6));
    TEST_EQFISH(r.t, 0.275);

    // tangent edge skim from corner
    r = intersect(circle{{0,2},1}, {8,0}, ax);
    TEST_EQ(r.impact, xy(3,1)); // hard to imagine this one not being exact
    TEST_EQXYISH(r.normal, xy(0,1));
    TEST_EQFISH(r.t, 0.375);

    // rotated face center hit along the box axis; normal rotated back to world
    r = intersect(circle{{-3,-4}, 1}, {6,8}, rot);
    TEST_EQXYISH(r.impact, xy(-0.6, -0.8));
    TEST_EQXYISH(r.normal, xy(-0.6, -0.8));
    TEST_EQFISH(r.t, 0.3);

    // Corner-arc case (above) re-centered on the box and rotated by rot's
    // (cos,sin)=(0.6,0.8).  Off-axis, asymmetric contact: impact and normal
    // are distinct directions with nonzero components on both box axes, so
    // this catches a missing/wrong local->world transform on either output
    // that the on-axis face-center case can't.  t is rotation-invariant.
    auto R = [](xy p) { return xy(0.6 * p.x() - 0.8 * p.y(),
                                  0.8 * p.x() + 0.6 * p.y()); };
    r = intersect(circle{R({-4, 1.6}), 1}, R({8, 0}), rot);
    TEST_EQXYISH(r.impact, R({-1, 1}));
    TEST_EQXYISH(r.normal, R({-0.8, 0.6}));
    TEST_EQFISH(r.t, 0.275);

    // from_axis agrees with explicit cos/sin
    r = intersect(circle{{-3,-4}, 1}, {6,8}, obb::from_axis({0,0}, {1,1}, {3,4}));
    TEST_EQXYISH(r.normal, xy(-0.6, -0.8));
    TEST_EQFISH(r.t, 0.3);

    // penetration at start, off-face: push out perpendicular to nearest face
    r = intersect(circle{{0, 1.5}, 1}, {2,0}, org);
    TEST_EQXYISH(r.impact, xy(0, 0.5));
    TEST_EQXYISH(r.normal, xy(0, 1));
    TEST_EQFISH(r.t, 0);
    TEST(r);

    // penetration with center inside the box: degenerate -delta fallback
    r = intersect(circle{{0, 0.5}, 1}, {2,0}, org);
    TEST_EQXYISH(r.impact, xy(1, 0.5));
    TEST_EQXYISH(r.normal, xy(-1, 0));
    TEST_EQFISH(r.t, 0);

    // corner notch: inside inflated AABB, outside rounded shape, sweeping in
    r = intersect(circle{{1.8, 1.8}, 1}, {-1,-1}, org);
    TEST_EQXYISH(r.impact, xy(1,1));
    TEST_EQXYISH(r.normal, xy(math::sqrt2/2, math::sqrt2/2));
    TEST_EQFISH(r.t, 0.8 - math::sqrt2/2);

    // corner notch sweeping away: t<0 root rejected, not a forward entry
    r = intersect(circle{{1.8, 1.8},1}, {1,1}, org);
    TEST_NOT(r);
    TEST_NOT(r.impact);
    TEST_NOT(r.normal);
    TEST_NAN(r.t);
    TEST_REPR(r, "nil");

    // zeroish length: static rounded-rect overlap
    r = intersect(circle{{1.5, 0}, 1}, {0,0}, org);
    TEST_EQXYISH(r.impact, xy(0.5, 0));
    TEST_EQXYISH(r.normal, xy(1, 0));
    TEST_EQFISH(r.t, 0);
    TEST_NOT(intersect(circle{{2.5, 0}, 1}, {0,0}, org));

    // miss
    TEST_NOT(intersect(circle{{0,5}, 1}, {8,0}, ax));
    TEST_NOT(intersect(circle{{0,3}, 1}, {8,0}, ax));
}

DEFINE_TEST(intersect_ray_triangle) {
    triangle tri{{0,0}, {4,0}, {0,3}};

    TEST_EQ(intersect({2,-2}, {0,4}, tri), 0.5); // bottom edge
    TEST_EQ(intersect({2,1}, {1,0}, tri), 0); // started inside
    TEST_EQ(intersect({4,0}, {1,1}, tri), 0); // on a vertex
    TEST_NAN(intersect({4.01, -2}, {0,4}, tri)); // near miss
    TEST_EQ(intersect({2,1}, {0,0}, tri), 0); // zeroish length, static containment
    TEST_NAN(intersect({2,-1}, {0,0}, tri)); // or not
}

DEFINE_TEST(intersect_circle_triangle) {
    const float sqrt3 = std::sqrt(3.f);
    triangle tri{{0,0}, {4,0}, {0,3}}; // 3-4-5 right triangle
    triangle rev{{0,0}, {0,3}, {4,0}}; // same, reversed winding

    intersection r = intersect(circle{{2,-4}, 1}, {0,4}, tri);
    TEST_EQ(r.impact, xy(2,0));
    TEST_EQXYISH(r.normal, xy(0,-1));
    TEST_EQFISH(r.t, 0.75);

    // face hit, hypotenuse, straight down its normal
    r = intersect(circle{{5, 5.5}, 1}, {-4.8, -6.4}, tri);
    TEST_EQXYISH(r.impact, xy(2, 1.5));
    TEST_EQXYISH(r.normal, xy(0.6, 0.8));
    TEST_EQFISH(r.t, 0.5);

    // winding independence: reversed vertex order, same answer
    r = intersect(circle{{5, 5.5}, 1}, {-4.8, -6.4}, rev);
    TEST_EQXYISH(r.impact, xy(2, 1.5));
    TEST_EQXYISH(r.normal, xy(0.6, 0.8));
    TEST_EQFISH(r.t, 0.5);

    // vertex cap at B: diagonal approach; hypotenuse's face entry is off-span
    r = intersect(circle{{8,3}, 1}, {-4,-4}, tri);
    TEST_EQXYISH(r.impact, xy(4,0));
    TEST_EQXYISH(r.normal, xy(1,0)); // as if wall at x=4
    TEST_EQFISH(r.t, 0.75);

    // penetration, center inside the solid: degenerate -delta fallback
    r = intersect(circle{{2, 0.5}, 1}, {1,0}, tri);
    TEST_EQXYISH(r.impact, xy(3, 0.5));
    TEST_EQXYISH(r.normal, xy(-1, 0));
    TEST_EQFISH(r.t, 0);

    // penetration, center outside but within R: push out through nearest face
    r = intersect(circle{{2, -0.5}, 1}, {1,0}, tri);
    TEST_EQXYISH(r.impact, xy(2, 0.5));
    TEST_EQXYISH(r.normal, xy(0, -1));
    TEST_EQFISH(r.t, 0);

    // vertex notch: inside the offset wedge, outside the cap
    r = intersect(circle{{5.5, -0.5}, 1}, {1,0}, tri); // sweeping away
    TEST_REPR(r, "nil");
    r = intersect(circle{{5.5, -0.5}, 1}, {-4,0}, tri); // sweeping in: cap entry
    TEST_EQXYISH(r.impact, xy(4,0));
    TEST_EQXYISH(r.normal, xy(sqrt3/2, -0.5));
    TEST_EQFISH(r.t, (3 - sqrt3) / 8);

    // contact at finish / stops short
    r = intersect(circle{{2,-4}, 1}, {0,3}, tri);
    TEST_EQXYISH(r.impact, xy(2,0));
    TEST_EQXYISH(r.normal, xy(0,-1));
    TEST_EQFISH(r.t, 1);
    TEST_REPR(intersect(circle{{2,-4}, 1}, {0,2}, tri), "nil");

    // zeroish length: static overlap, face region and vertex region
    r = intersect(circle{{2, -0.5}, 1}, {0,0}, tri);
    TEST_EQXYISH(r.normal, xy(0,-1));
    TEST_EQFISH(r.t, 0);
    r = intersect(circle{{4.5, -0.5}, 1}, {0,0}, tri);
    TEST_EQXYISH(r.normal, xy(math::sqrt2/2, -math::sqrt2/2));
    TEST_EQFISH(r.t, 0);
    TEST_NAN(intersect(circle{{2,-3}, 1}, {0,0}, tri).t);

    // obtuse isosceles (30-30-120): straight down onto the wide apex's cap
    triangle obt{{0,0}, {6,0}, {3, sqrt3}};
    r = intersect(circle{{3,5}, 1}, {0,-4}, obt);
    TEST_EQXYISH(r.impact, xy(3, sqrt3));
    TEST_EQXYISH(r.normal, xy(0,1));
    TEST_EQFISH(r.t, (4 - sqrt3) / 4);
}

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
