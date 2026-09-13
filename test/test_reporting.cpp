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

#define TEST_ASSERT_FITS(T) TAF1(std::make_signed_t<T>, std::make_unsigned_t<T>)

#define TAF1(S, U) TAF2(S,                              \
                        std::numeric_limits<S>::min(),  \
                        std::numeric_limits<S>::max(),  \
                        U,                              \
                        std::numeric_limits<U>::min(),  \
                        std::numeric_limits<U>::max())

#define TAF2(S, smin, smax, U, umin, umax)              \
    do {                                                \
        TAF_RANGE(S, smin, smax);                       \
        TAF_RANGE(U, umin, umax);                       \
        TAF_SIGNAGE(S, smin, smax, U, umin, umax);      \
    } while (false)

#define TAF_RANGE(T, min, max)                                  \
    do {                                                        \
        ASSERT_FITS(min, T);                                    \
        ASSERT_FITS(0, T);                                      \
        ASSERT_FITS(max, T);                                    \
        ASSERT_FITS(int64_t(min), T);                           \
                                                                \
        if constexpr (sizeof(T) < 8) {                          \
            ASSERT_FITS(int64_t(max), T);                       \
            TEST_THROW(ASSERT_FITS(int64_t(min) - 1, T));       \
            TEST_THROW(ASSERT_FITS(int64_t(max) + 1, T));       \
        }                                                       \
    } while (false)

#define TAF_SIGNAGE(S, smin, smax, U, umin, umax)               \
    do {                                                        \
        TEST_THROW(ASSERT_FITS(smin, U));                       \
        ASSERT_FITS(smax, U);                                   \
        ASSERT_FITS(umin, S);                                   \
        TEST_THROW(ASSERT_FITS(umax, S));                       \
    } while(false)

DEFINE_TEST(fits) {
    TEST_ASSERT_FITS(int8_t);
    TEST_ASSERT_FITS(int16_t);
    TEST_ASSERT_FITS(int32_t);
    TEST_ASSERT_FITS(int64_t);
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
