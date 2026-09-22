// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include <core/repr.hpp>

#include <core/test/tests.hpp>

namespace hjx {

DEFINE_TEST(honest_float) {
    rand_t rand(test::harness::seed());

    for (int i = 0; i < 999'999; i++) {
        float a = std::bit_cast<float>(uint32_t(rand()));
        string s = TO_STRING(honest_float(a));
        float b;
        auto [ptr, ec] = std::from_chars(&*s.begin(), &*s.end(), b);
        TEST(ec == std::errc{}); // WTF C++.  no errc::operator<<?
        TEST_EQ(ptr, &*s.end());
        TEST_MSG((std::isnan(a) && std::isnan(b)) || a == b,
                 frepr(a) << " vs " << frepr(b));
    }
}

DEFINE_TEST(as_hex) {
    TEST_REPR(as_hex{0}, "0x0000'0000'0000'0000");
    TEST_REPR(as_hex{1}, "0x0000'0000'0000'0001");
    TEST_REPR(as_hex{0x0123'4567'89ab'cdef}, "0x0123'4567'89ab'cdef");
    TEST_REPR(as_hex{0xffff'ffff'ffff'ffff}, "0xffff'ffff'ffff'ffff");
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:repr", argv);
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
