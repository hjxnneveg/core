// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Joshua C Marshall

#include <core/hex/node.hpp>

#include <core/test/tests.hpp>

namespace hjx {

DEFINE_TEST(nodes) {
    hex::foreach_nodeshift([](hex::nodecoords c) {
        TEST_EQ(c.chief(), hex::nodecoords(0,0));
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
