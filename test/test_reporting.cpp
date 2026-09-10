// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include <core/reporting.hpp>

#include <core/test/tests.hpp>

namespace hjx {

DEFINE_TEST(assert) {
    int x = 0;
    ASSERT_EQ(x++, 0);
    TEST_EQ(x, ASSERTS_ON ? 1 : 0);

    x = 0;
    ASSERT_IMPLY(false, x++);
    TEST_EQ(x, 0);

    ASSERT_EQFISH(2, 2.00001);

    TEST_THROW([]{ ASSERT(false); }());

#if !ASSERTS_ON
    ASSERT(false);
#endif
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:reporting", argv);
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
