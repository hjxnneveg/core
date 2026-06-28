// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/decimal.hpp"

#include "core/test/tests.hpp"

namespace hjx {

template<typename T>
void test_decimal_common() {
    using Rep = decltype(T{}.storage);
    constexpr bool is_signed = std::is_signed_v<Rep>;
    constexpr unsigned exp = T::scale == 1 ? 0 : T::scale == 10 ? 1 : 2;

    TEST(T{}.nilp());
    TEST(T::nil().nilp());
    TEST_EQ(T{}, T{});
    TEST_NE(T{}, T{0});
    TEST(T{0/0.f}.nilp());
    TEST(T{1/0.f}.nilp());
    TEST(T{-1/0.f}.nilp());

    TEST_EQ(T::max(), T::from_repr(T::max_repr));
    TEST_EQ(T::min(), T::from_repr(T::min_repr));
    if (is_signed) TEST_EQ(T::min(), T{-T::max().get()});

    TEST(T{T::max().get() + 1.f}.nilp());
    if (!is_signed) TEST(T{-1.f}.nilp());

    TEST(T{0} < T::max());
    if (is_signed) TEST(T::min() < T{0});
    TEST_NOT(T{} < T{0});
    TEST_NOT(T{} > T{0});
    TEST_NOT(T{} <= T{0});
    TEST_NOT(T{} >= T{0});
    TEST_NOT(T{} == T{0});
    TEST((T{} <=> T{0}) == std::partial_ordering::unordered);

    TEST_LT(sizeof(T), sizeof(int));
    for (int i = std::numeric_limits<Rep>::min();
         i <= std::numeric_limits<Rep>::max();
         i++)
    {
        T v = T::from_repr(static_cast<Rep>(i));
        if (v.nilp()) continue;
        float f = v.get();
        TEST_NOT(std::isnan(f));
        TEST_EQ(v, T{f});
    }

    TEST_REPR(T{}, "nil");

    auto test_token = [](const char *s, T expected) {
        T actual = T::parse(s);
        TEST_MSG(actual == expected,
                 "'" << s << "' actual: " << actual << ", expected: " << expected);
    };

    test_token("", T{});
    test_token(".", T{});
    test_token("a", T{});
    test_token(" 1", 1);

    test_token("0", 0);
    test_token("00", 0);

    if (exp >= 1) {
        test_token("0.0", 0);
        test_token("1.", 1);
        test_token("09.0", 9);  // not octal
    }

    if (exp >= 2) {
        test_token("0.00", 0);
        test_token("00.00", 0);
    }

    if      (exp == 0) test_token("1.0", T{});
    else if (exp == 1) test_token("1.00", T{});
    else if (exp == 2) test_token("1.000", T{});

    if (is_signed) {
        test_token("-1", -1);
        test_token("-", T{});
        test_token("-.", T{});
    }
    else {
        test_token("-1", T{});
    }

    if (exp >= 1) test_token(".0", 0);
    if (exp >= 2) test_token(".00", 0);
    if (is_signed && exp >= 1) test_token("-.1", -0.1);

    for (int i = std::numeric_limits<Rep>::min();
         i <= std::numeric_limits<Rep>::max(); i++)
    {
        T c = T::from_repr(static_cast<Rep>(i));
        if (c.nilp()) continue;
        std::string s = TO_STRING(c);
        TEST_EQ(T::parse(s), c);
    }
}

DEFINE_TEST(decimal) {
    test_decimal_common<decimal<uint8_t,  0>>();
    test_decimal_common<decimal<uint8_t,  1>>();
    test_decimal_common<decimal<uint8_t,  2>>();
    test_decimal_common<decimal<int8_t,   0>>();
    test_decimal_common<decimal<int8_t,   1>>();
    test_decimal_common<decimal<int8_t,   2>>();
    test_decimal_common<decimal<uint16_t, 0>>();
    test_decimal_common<decimal<uint16_t, 1>>();
    test_decimal_common<decimal<uint16_t, 2>>();
    test_decimal_common<decimal<int16_t,  0>>();
    test_decimal_common<decimal<int16_t,  1>>();
    test_decimal_common<decimal<int16_t,  2>>();

    TEST_EQ(cent16{6.424999}.get(), 6.42f);
    TEST_EQ(cent16{6.4249999}.get(), 6.43f);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:decimal", argv);
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
