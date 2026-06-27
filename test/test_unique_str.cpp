// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/unique_str.hpp"

#include "tests/tests.hpp"

namespace hjx {

DEFINE_TEST(unique_str) {
    unique_str a;
    TEST_NOT(a.get());
    TEST_REPR(a, "<null>");

    a = unique_str("one");
    TEST_EQ(a.get(), string("one"));
    TEST_REPR(a, "one");

    unique_str b("two");
    TEST_EQ(b.get(), string("two"));
    TEST_REPR(b, "two");

    b = std::move(a);
    TEST_NOT(a.get());
    TEST_REPR(a, "<null>");
    TEST_EQ(b.get(), string("one"));
    TEST_REPR(b, "one");
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:unique_str", argv);
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
