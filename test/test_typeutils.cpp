// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include <core/typeutils.hpp>

#include <core/test/tests.hpp>

namespace hjx {

DEFINE_TEST(cmp) {
    TEST( cmp::eq(1.f, 1u));
    TEST( cmp::eq(1.f, 1.0));
    TEST( cmp::eq(int8_t(-1), int64_t(-1)));
    TEST(!cmp::eq(-1ll, -1ull));

    TEST(!cmp::ne(1.f, 1u));
    TEST(!cmp::ne(1.f, 1.0));
    TEST(!cmp::ne(int8_t(-1), int64_t(-1)));
    TEST( cmp::ne(-1ll, -1ull));

    TEST(!cmp::ge(-1, 1u));
    TEST( cmp::ge(1u, -1));
    TEST( cmp::ge(-1, -1ll));
    TEST( cmp::ge(int8_t(-1), int64_t(-1)));

    TEST(!cmp::gt(-1, 1u));
    TEST( cmp::gt(1u, -1));
    TEST(!cmp::gt(-1, -1ll));
    TEST(!cmp::gt(int8_t(-1), int64_t(-1)));

    TEST( cmp::le(-1, 1u));
    TEST(!cmp::le(1u, -1));
    TEST( cmp::le(-1, -1ll));
    TEST( cmp::le(int8_t(-1), int64_t(-1)));

    TEST( cmp::lt(-1, 1u));
    TEST(!cmp::lt(1u, -1));
    TEST(!cmp::lt(-1, -1ll));
    TEST(!cmp::lt(int8_t(-1), int64_t(-1)));
}


DEFINE_TEST(countof) {
    int  i1[1]; TEST_EQ(countof(i1), 1);
    int  i2[2]; TEST_EQ(countof(i2), 2);
    char c1[1]; TEST_EQ(countof(c1), 1);
    char c2[2]; TEST_EQ(countof(c2), 2);
}


void picky_inteater(std::integral auto T) {}

void flexible_inteater(int) {}

DEFINE_TEST(callable) {
    TEST_THROW(throw 6);

    TEST(IS_CALLABLE(picky_inteater, int));
    TEST(!IS_CALLABLE(picky_inteater, float));
    TEST(IS_CALLABLE(flexible_inteater, int));
    TEST(IS_CALLABLE(flexible_inteater, float));
    TEST(!IS_CALLABLE(inteater_with_a_typo, int));
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:typeutils", argv);
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
