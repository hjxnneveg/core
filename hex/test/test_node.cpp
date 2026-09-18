// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include <core/hex/node.hpp>

#include <core/test/tests.hpp>

namespace hjx {

void test_eq(hex::nodecoords a, hex::nodecoords b) { TEST_MSG(a == b, a << " vs " << b); }

DEFINE_TEST(nodes) {
    hex::foreach_nodeshift([](hex::nodecoords c) {
        test_eq(c.chief(), {0,0});
        TEST(hex::nodecoords(0,0).owns(c));
        TEST(hex::is_nodeshift(c.q(), c.r()));
    });

    hex::foreach_nodeshift([](int, hex::nodecoords) {});
    hex::foreach_nodeshift([](size_t, hex::nodecoords) {});

    TEST(!hex::is_nodeshift(-2,-2));
    TEST(!hex::is_nodeshift( 0,-3));
    TEST(!hex::is_nodeshift( 2,-4));
    TEST(!hex::is_nodeshift( 3,-3));
    TEST(!hex::is_nodeshift( 4,-2));
    TEST(!hex::is_nodeshift( 3, 0));
    TEST(!hex::is_nodeshift( 2, 2));

    TEST(!hex::is_nodeshift(0, 1));
    TEST(!hex::is_nodeshift(7, 7));
}

void test_indices(auto &&f) {
    unsigned mask = 0;

    hex::foreach_nodeshift([&](hex::nodecoords c) {
        unsigned u = 1 << f(c.q(), c.r());
        TEST_MSG(!(mask & u), c);
        mask |= u;
    });

    TEST_EQ(mask, 0xfff);
}

DEFINE_TEST(unique_idx) {
    test_indices(hex::impl::coord_index);
    test_indices(hex::impl::shift_index);
}

void test_owner(hex::nodecoords offset) {
    auto f = [](hex::nodecoords chief, hex::nodecoords sub){ TEST(chief.owns(sub)); };

    f(offset.shifted( 0,-6), offset.shifted(-2,-2));
    f(offset.shifted( 0,-6), offset.shifted( 0,-3));
    f(offset.shifted( 6,-6), offset.shifted( 2,-4));

    f(offset.shifted( 0, 0), offset.shifted(-3, 0));
    f(offset.shifted( 0, 0), offset.shifted(-1,-1));
    f(offset.shifted( 0, 0), offset.shifted( 1,-2));
    f(offset.shifted( 6,-6), offset.shifted( 3,-3));

    f(offset.shifted( 0, 0), offset.shifted(-4, 2));
    f(offset.shifted( 0, 0), offset.shifted(-2, 1));
    f(offset.shifted( 0, 0), offset.shifted( 0, 0));
    f(offset.shifted( 0, 0), offset.shifted( 2,-1));
    f(offset.shifted( 6,-6), offset.shifted( 4,-2));

    f(offset.shifted( 0, 0), offset.shifted(-3, 3));
    f(offset.shifted( 0, 0), offset.shifted(-1, 2));
    f(offset.shifted( 0, 0), offset.shifted( 1, 1));
    f(offset.shifted( 6, 0), offset.shifted( 3, 0));

    f(offset.shifted( 0, 0), offset.shifted(-2, 4));
    f(offset.shifted( 0, 0), offset.shifted( 0, 3));
    f(offset.shifted( 6, 0), offset.shifted( 2, 2));
}

DEFINE_TEST(owner) {
    test_owner({-6, 0});
    test_owner({-6, 6});
    test_owner({ 0,-6});
    test_owner({ 0, 0});
    test_owner({ 0, 6});
    test_owner({ 6,-6});
    test_owner({ 6, 0});
}

DEFINE_TEST(bearing) {
    using namespace hex;

    for (int ock = 0; ock < 12; ock++)
        for (int fore = -6; fore <= 6; fore++)
            for (int sb = -6; sb <= 6; sb++)
                if ((fore + sb) & 1)
                    TEST_THROW_MSG(bearing(ock, fore, sb),
                                   ock << " " << fore << " " << sb << " "
                                   << bearing(ock, fore, sb));
                else {
                    nodecoords a = bearing(ock, fore, sb);
                    TEST(a);
                    nodecoords b = bearing((ock + 6) % 12, -fore, -sb);
                    TEST_EQ(a, b);

                    TEST_EQ(reach(ock, a), fore);
                    TEST_EQ(reach((ock + 3) % 12, a), sb);
                }

    for (int ock = 0; ock < 12; ock++)
        test_eq(bearing(ock, 0, 0), {0,0});

    TEST_THROW(bearing(-1, 0, 0));
    TEST_THROW(bearing(13, 0, 0));

    // the clock ring
    test_eq(bearing( 0, 2, 0), { 0,-3});
    test_eq(bearing( 1, 2, 0), { 1,-2});
    test_eq(bearing( 2, 2, 0), { 3,-3});
    test_eq(bearing( 3, 2, 0), { 2,-1});
    test_eq(bearing( 4, 2, 0), { 3, 0});
    test_eq(bearing( 5, 2, 0), { 1, 1});
    test_eq(bearing( 6, 2, 0), { 0, 3});
    test_eq(bearing( 7, 2, 0), {-1, 2});
    test_eq(bearing( 8, 2, 0), {-3, 3});
    test_eq(bearing( 9, 2, 0), {-2, 1});
    test_eq(bearing(10, 2, 0), {-3, 0});
    test_eq(bearing(11, 2, 0), {-1,-1});


    test_eq(bearing(0, 1, 1), { 1,-2});
    test_eq(bearing(6,-1,-1), { 1,-2});
    test_eq(bearing(0, 0, 2), { 2,-1});
    test_eq(bearing(0,-2, 4), { 4, 1});
    test_eq(bearing(1, 2, 0), { 1,-2});
    test_eq(bearing(1, 0, 2), { 3, 0});
    test_eq(bearing(1, 0,-2), {-3, 0});
    test_eq(bearing(1,-1,-1), {-2, 1});
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:hex:node", argv);
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
