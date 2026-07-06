// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include <core/hex/grid.hpp>

#include <core/test/tests.hpp>

namespace hjx {

DEFINE_TEST(hexgrid) {
    hex::grid<int> g{5};
    TEST_REPR(g, "[hexgrid int 5]");
    TEST_EQ(g.count(), 19);

    g.foreach([&](qrs pos, int &hex) {
        TEST_MSG(&g.to_hex(pos) == &hex, pos << " " << hex);
        TEST_MSG(g.to_qrs(hex) == pos, pos << " " << hex);
    });

    g.foreach([&](qrs pos, int &hex) { hex = g.to_scalar(pos) + 100; });

    int i = 100;
    g.foreach([&](int &hex) { TEST_EQ(hex, i++); });

    i = 100;
    for (int &hex : g) TEST_EQ(hex, i++);

    g.wipe();
    g.foreach([&](int &hex) { TEST_EQ(hex, 0); });

    hex::grid<int> big{555};
    TEST_REPR(big, "[hexgrid int 555]");
    TEST_EQ(big.count(), 231019);

    TEST_THROW(hex::grid<int>{6});
}

DEFINE_TEST(bounds) {
    hex::grid<int> g{5};

    for (float q = -3; q <= 3; q++)
        for (float r = -3; r <= 3; r++) {
            qrs pos{q, r};

            if (pos.manhattan() <= 2) {
                TEST_MSG(g.in_bounds(pos), pos);

                if (pos.manhattan() == 2)
                    TEST_MSG(g.border(pos), pos);
                else
                    TEST_MSG(!g.border(pos), pos);
            }
            else {
                TEST_MSG(!g.in_bounds(pos), pos);
                TEST_THROW(g.to_scalar(pos));
            }
        }
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:hex:grid", argv);
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
