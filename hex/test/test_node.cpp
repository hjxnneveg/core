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
