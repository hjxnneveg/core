// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/finale.hpp"

#include "tests/tests.hpp"

namespace hjx {

finale finale_helper() { return finale([] { TEST_EQ(0, 1); }); }

void finale_nonlambda_cb() {}

DEFINE_TEST(finale) {
    int x = 0;
    {
        finale _([&] { x = 1; });
        TEST_EQ(x, 0);
    }
    TEST_EQ(x, 1);

    x = 1;
    {
        finale one([&] { x += 1; });
        finale two([&] { x *= 2; });
    }
    TEST_EQ(x, 3);

    x = 0;
    {
        finale one([&] { x++; });
        finale two = std::move(one);
        TEST_EQ(x, 0);
    }
    TEST_EQ(x, 1);

    x = 0;
    {
        finale outer([] { TEST_EQ(0, 1); });
        {
            finale inner([&] { x = 42; });
            outer.disarm();
            outer = std::move(inner);
            TEST_EQ(x, 0);
        }
        TEST_EQ(x, 0);
    }
    TEST_EQ(x, 42);

    {
        finale bomb([] { TEST_EQ(0, 1); });
        bomb.disarm();
    }

    x = 0;
    {
        finale one([&] { x += 1; });
        finale two([&] { x += 2; });
        TEST_EQ(x, 0);
        two = std::move(one);
        TEST_EQ(x, 2);
    }
    TEST_EQ(x, 3);

    finale bomb = finale_helper();
    bomb.disarm();

    {
        finale _(finale_nonlambda_cb);
    }
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:finale", argv);
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
