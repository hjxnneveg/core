// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/direction.hpp"

#include "core/test/tests.hpp"

namespace hjx {

#define TEST_UNIQUE(...)                                        \
    ([&] {                                                      \
        const auto arr = std::array{__VA_ARGS__};               \
        const std::unordered_set set(arr.begin(), arr.end());   \
        TEST_EQ(arr.size(), set.size());                        \
    }())

DEFINE_TEST(direction) {
    // edges
    TEST_UNIQUE(direction::none,
                direction::north,
                direction::ne,
                direction::se,
                direction::south,
                direction::sw,
                direction::nw);

    // vertices
    TEST_UNIQUE(direction::none,
                direction::east,
                direction::se,
                direction::sw,
                direction::west,
                direction::nw,
                direction::ne);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:direction", argv);
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
