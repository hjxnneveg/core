// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/token.hpp"

#include "tests/tests.hpp"

namespace hjx {

DEFINE_TEST(intern) {
    char a1[] = "a";
    char a2[] = "a";
    char b[] = "b";
    TEST_EQ(intern(a1), intern(a2));
    TEST_NE(intern(a1), a1);
    TEST_NE(intern(a1), intern(b));

    char e1[] = "";
    char e2[] = "";
    TEST_EQ(intern(e1), intern(e2));
    TEST_NE(intern(e1), intern(b));
}

DEFINE_TEST(token) {
    char a1[] = "a"; token ta1(a1);
    char a2[] = "a"; token ta2(a2);
    char b[] = "b";  token tb(b);

    TEST_EQ(ta1, ta1);
    TEST_EQ(ta1, ta2);
    TEST_NE(tb, ta1);

    token te1(nullptr);
    token te2;

    TEST_EQ(te1, te2);
    TEST_NE(te1, ta1);
    TEST_NE(te2, ta1);

    TEST_EQ(ta1, token(string("a")));
    TEST_EQ(ta1, token(string_view("a")));
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:token", argv);
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
