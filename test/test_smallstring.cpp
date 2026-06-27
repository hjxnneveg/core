// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/smallstring.hpp"

#include "tests/tests.hpp"

namespace hjx {

DEFINE_TEST(smallstring) {
    smallstring<23> s;
    TEST_EQ(s.size(), 0);
    TEST_NOT(s.full());
    TEST_EQ(s.sv(), "");
    TEST_REPR(s, "");

    s = "qwerty";
    TEST_EQ(s.size(), 6);
    TEST_NOT(s.full());
    TEST_EQ(s.sv(), "qwerty");
    TEST_REPR(s, "qwerty");

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
    s = s;
    TEST_REPR(s, "qwerty");
#pragma clang diagnostic pop

    s = smallstring<23>("azerty");
    TEST_EQ(s.size(), 6);
    TEST_EQ(s.sv(), "azerty");
    TEST_REPR(s, "azerty");

    s = "";
    TEST_EQ(s.size(), 0);
    TEST_REPR(s, "");

    TEST_EQ(s.capacity, 23);
    s = "12345678901234567890123";
    TEST_EQ(s.size(), 23);
    TEST(s.full());
    TEST_REPR(s, "12345678901234567890123");
    TEST_THROW(smallstring<23>("123456789012345678901234"));

    s = unsigned(0);
    TEST_REPR(s, "0x0000000000000000");
    TEST_NOT(s.full());

    s = 0xdecafc0ffee15bad;
    TEST_REPR(s, "0xdecafc0ffee15bad");
    TEST_NOT(s.full());

    s = "";
    s.append('x');
    TEST_EQ(s.size(), 1);
    TEST_REPR(s, "x");
    s.append('y');
    s.append('z');
    TEST_EQ(s.size(), 3);
    TEST_REPR(s, "xyz");

    s = "";
    for (size_t i = 0; i < smallstring<23>::capacity; i++) s.append('.');
    TEST(s.full());
    TEST_EQ(s.size(), 23);
    TEST_REPR(s, ".......................");
    TEST_THROW(s.append('.'));

    smallstring<23> a("abc");
    smallstring<23> b("abc");
    smallstring<23> c("abd");
    smallstring<23> d("ab");
    TEST_EQ(a, b);
    TEST(a <=> c == std::strong_ordering::less);
    TEST(c <=> a == std::strong_ordering::greater);
    TEST(d <=> a == std::strong_ordering::less);

    a = "text";
    b = a;
    TEST_EQ(a, b);
    TEST_REPR(b, "text");

    s = "café";
    TEST_EQ(s.size(), 5);
    TEST_REPR(s, "café");
    TEST_NE(s, smallstring<23>("café")); // don't @ me
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:smallstring", argv);
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
