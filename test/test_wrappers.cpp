// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/wrappers.hpp"

#include "tests/tests.hpp"

#include <set>
#include <unordered_set>

namespace hjx {

DEFINE_TEST(wrappers) {
    TEST_EQ(multiplier{6}, multiplier{6});
    TEST_EQ(multiplier{0}, 0);

    std::set<multiplier> c1;
    std::unordered_set<multiplier> c2;

    // todo
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:wrappers", argv);
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
