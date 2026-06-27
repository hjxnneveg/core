// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/angle.hpp"

#include "tests/tests.hpp"

namespace hjx {

DEFINE_TEST(angle) {
    constexpr float pi = std::numbers::pi_v<float>;

    TEST_EQ(  0.0_deg,    0.0_deg);
    TEST_EQ(  0.0_deg,   -0.0_deg);
    TEST_EQ(  0.0_deg,  360.0_deg);
    TEST_EQ(180.0_deg,  180.0_deg);
    TEST_EQ(180.0_deg, -180.0_deg);
    TEST_EQ(-90.0_deg,  270.0_deg);

    TEST_REPR(0.0_deg, -0.0_deg);

    // These fail with -O3 -ffast-math
    TEST_EQ(  0.0_deg, angle::radians(0));
    TEST_EQ( 30.0_deg, angle::radians(   pi/6));
    TEST_EQ( 60.0_deg, angle::radians(   pi/3));
    TEST_EQ( 90.0_deg, angle::radians(   pi/2));
    TEST_EQ(120.0_deg, angle::radians( 2*pi/3));
    TEST_EQ(150.0_deg, angle::radians( 5*pi/6));
    TEST_EQ(180.0_deg, angle::radians(   pi  ));
    TEST_EQ(210.0_deg, angle::radians(-5*pi/6));
    TEST_EQ(240.0_deg, angle::radians(-2*pi/3));
    TEST_EQ(270.0_deg, angle::radians(  -pi/2));
    TEST_EQ(300.0_deg, angle::radians(  -pi/3));
    TEST_EQ(330.0_deg, angle::radians(  -pi/6));

    TEST_EQ(1.5_deg + 2.5_deg, 4.0_deg);
    TEST_EQ(1.5_deg - 2.5_deg, -1.0_deg);

    TEST_EQ( 60.0_deg / 2,  30.0_deg);
    TEST_EQ(120.0_deg / 2,  60.0_deg);
    TEST_EQ(180.0_deg / 2,  90.0_deg);
    //TEST_EQ(240.0_deg / 2, -60.0_deg);
    //TEST_EQ(300.0_deg / 2, -30.0_deg);

    TEST_EQ(oclock(-1), angle::degrees(-120));
    TEST_EQ(oclock( 0), angle::degrees( -90));
    TEST_EQ(oclock( 1), angle::degrees( -60));
    TEST_EQ(oclock( 2), angle::degrees( -30));
    TEST_EQ(oclock( 3), angle::degrees(   0));
    TEST_EQ(oclock( 4), angle::degrees(  30));
    TEST_EQ(oclock( 5), angle::degrees(  60));
    TEST_EQ(oclock( 6), angle::degrees(  90));
    TEST_EQ(oclock( 7), angle::degrees( 120));
    TEST_EQ(oclock( 8), angle::degrees( 150));
    TEST_EQ(oclock( 9), angle::degrees( 180));
    TEST_EQ(oclock(10), angle::degrees(-150));
    TEST_EQ(oclock(11), angle::degrees(-120));
    TEST_EQ(oclock(12), angle::degrees( -90));
    TEST_EQ(oclock(13), angle::degrees( -60));
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:angle", argv);
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
