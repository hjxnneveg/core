// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/mass.hpp"

#include "core/test/tests.hpp"

namespace hjx {

DEFINE_TEST(mass) {
    TEST_EQ(mass_t().grams(), 0);
    TEST_EQ(mass_t(6, mass_t::GRAMS).grams(), 6);
    TEST_EQ(mass_t::max().grams(), 0xffff'ffff);
    TEST_EQ(mass_t::min().grams(), 0);

    TEST_THROW(mass_t(3e30, mass_t::GRAMS));

    TEST_EQ(0.0_kg .grams(), 0);
    TEST_EQ(0.001_kg .grams(), 1);
    TEST_EQ(1.0_kg .grams(), 1000);

    TEST_EQ(1.001499_kg .grams(), 1001);
    TEST_EQ(1.0015_kg .grams(), 1002);
    TEST_EQ(1.002499_kg .grams(), 1002);
    TEST_EQ(1.0025_kg .grams(), 1003);

    TEST_REPR(1.000_kg, "1kg");
    TEST_REPR(1.004_kg, "1.004kg");
    TEST_REPR(1.030_kg, "1.03kg");
    TEST_REPR(1.034_kg, "1.034kg");
    TEST_REPR(1.200_kg, "1.2kg");
    TEST_REPR(1.204_kg, "1.204kg");
    TEST_REPR(1.230_kg, "1.23kg");
    TEST_REPR(1.234_kg, "1.234kg");

    auto repr = [](mass_t::type grams) {
        std::ostringstream ss;
        mass_t(grams, mass_t::GRAMS).dumpfix(ss);
        return ss.str();
    };

    TEST_EQ(repr(       0), "0.000kg");
    TEST_EQ(repr(     999), "0.999kg");
    TEST_EQ(repr(    1000), "1.000kg");
    TEST_EQ(repr(    9999), "9.999kg");
    TEST_EQ(repr(   10000), "10.00kg");
    TEST_EQ(repr(   99999), "99.99kg");
    TEST_EQ(repr(  100000), "100.0kg");
    TEST_EQ(repr(  999999), "999.9kg");
    TEST_EQ(repr( 1000000), "1000.kg");
    TEST_EQ(repr( 9999999), "9999.kg");
    TEST_EQ(repr(10000000), "10000kg");
    TEST_EQ(repr(99999999), "99999kg");
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:mass", argv);
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
