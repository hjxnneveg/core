// SPDX-License-Identifier: MIT
// Copyright (c) 2025-2026 Joshua C Marshall

#include "core/modval.hpp"

#include "tests/tests.hpp"

namespace hjx {

DEFINE_TEST(modval) {
    constexpr auto mult = [](int val, const vector<float> &mods) {
        double acc = 1;
        for (float e : mods) acc *= e;
        return int(val * acc);
    };

    modval<int, float, mult> v(2);
    TEST_EQ(v.get(), 2);

    v.add(1.31);
    TEST(v.exists(1.31));
    TEST(!v.exists(1.32));
    TEST_EQ(v.get(), 2);

    v.add(1.32);
    TEST(v.exists(1.31));
    TEST(v.exists(1.32));
    TEST_EQ(v.get(), 3);

    v.remove(1.31);
    TEST(!v.exists(1.31));
    TEST(v.exists(1.32));
    TEST_EQ(v.get(), 2);

    v.remove(1.32);
    TEST(!v.exists(1.31));
    TEST(!v.exists(1.32));
    TEST_EQ(v.get(), 2);

    v.add(1.31);
    v.add(1.32);
    v.remove(1.32);
    TEST(v.exists(1.31));
    TEST(!v.exists(1.32));
    TEST_EQ(v.get(), 2);
}

}

int main(int, const char *argv[]) {
    try {
        hjx::test::harness::exec("core:modval", argv);
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
